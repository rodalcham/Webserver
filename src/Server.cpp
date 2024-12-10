#include "../include/Webserv.hpp"
#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <algorithm>

extern std::atomic<bool> keepRunning;

HttpResponse generateDirectoryListing(const std::string& path) {
    std::cout << "[DEBUG] Generating directory listing for: " << path << "\n";
    std::string body = "<html><body><h1>Directory Listing for " + path + "</h1></body></html>";
    HttpResponse response;
    response.setBody(body);
    response.setHeader("Content-Length", std::to_string(body.size()));
    response.setHeader("Content-Type", "text/html");
    return response;
}



Server::Server(const std::vector<ServerBlock>& blocks) : serverBlocks(blocks) {
    if (serverBlocks.empty()) {
        throw std::runtime_error("No server blocks provided");
    }

    std::cout << "[DEBUG] Initializing server with " << serverBlocks.size() << " server blocks.\n";

    // Create the kqueue
    kq = kqueue();
    if (kq < 0) throw std::runtime_error("kqueue creation failed");

    // For each ServerBlock, create a listening socket
    for (size_t i = 0; i < serverBlocks.size(); ++i) {
        const ServerBlock& block = serverBlocks[i];
        std::cout << "[DEBUG] Setting up server block " << i << "\n";

        if (block.directive_pairs.find("listen") == block.directive_pairs.end()) {
            throw std::runtime_error("No listen directive found in server block " + std::to_string(i));
        }

        int port = std::stoi(block.directive_pairs.at("listen"));
        std::cout << "[DEBUG] Creating listening socket for port: " << port << "\n";

        int serverSock = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSock < 0) throw std::runtime_error("Socket creation failed");

        int flags = fcntl(serverSock, F_GETFL, 0);
        if (flags < 0 || fcntl(serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
            throw std::runtime_error("Failed to set non-blocking mode");

        int opt = 1;
        if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Failed to set socket options");

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
            throw std::runtime_error("Bind failed");

        if (listen(serverSock, 5) < 0)
            throw std::runtime_error("Listen failed");

        std::cout << "[DEBUG] Listening socket created and bound to port " << port << "\n";

        struct kevent event;
        EV_SET(&event, serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
            throw std::runtime_error("Failed to add server socket to kqueue");

        listenSockets.push_back(serverSock);
        socketToBlockMap[serverSock] = (ServerBlock*)&block;
    }
}

Server::~Server() {
    for (int sock : listenSockets) {
        close(sock);
    }
    close(kq);
    std::cout << "[DEBUG] Server destroyed. All sockets closed.\n";
}

void Server::run() {
    std::cout << "[DEBUG] Server is running. Awaiting connections...\n";
    while (keepRunning) {
        struct kevent eventList[1024];
        int eventCount = kevent(kq, nullptr, 0, eventList, 1024, nullptr);

        if (eventCount < 0) {
            if (errno == EINTR) continue; // Handle interrupts
            throw std::runtime_error("kevent() failed");
        }

        std::cout << "[DEBUG] Processing " << eventCount << " events.\n";

        for (int i = 0; i < eventCount; ++i) {
            int eventSock = (int)eventList[i].ident;

            if (socketToBlockMap.find(eventSock) != socketToBlockMap.end()) {
                std::cout << "[DEBUG] Accepting client on socket " << eventSock << "\n";
                acceptClient(eventSock);
            } else {
                std::cout << "[DEBUG] Handling client socket " << eventSock << "\n";
                handleClient(eventSock);
            }
        }
    }
}

void Server::acceptClient(int listeningSock) {
    int clientSock = accept(listeningSock, nullptr, nullptr);
    if (clientSock < 0) {
        std::cerr << "[ERROR] Failed to accept client connection.\n";
        return;
    }

    std::cout << "[DEBUG] Accepted client connection on socket " << clientSock << "\n";

    struct kevent event;
    EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0) {
        std::cerr << "[ERROR] Failed to add client socket to kqueue.\n";
        close(clientSock);
        return;
    }
}

void Server::handleClient(int clientSock) {
    char buffer[1024];
    ssize_t bytes = read(clientSock, buffer, sizeof(buffer));
    if (bytes <= 0) {
        std::cerr << "[DEBUG] Client disconnected or error occurred. Closing socket " << clientSock << "\n";
        close(clientSock);
        return;
    }

    std::string request(buffer, bytes);
    try {
        HttpRequest httpRequest = parseHttpRequest(request);

        // Match the server block and set the block_index in HttpRequest
        int i = matchServerBlock(httpRequest);
        if (i < 0) {
            std::cerr << "[ERROR] No matching server block found.\n";
            close(clientSock);
            return;
        }
        httpRequest.setBlockIndex(i); // Set the matched block index

        const auto& locations = serverBlocks[i].location_blocks;
        std::string matchedLocation = "/";
        size_t longestMatch = 0;

        // Updated matching logic
        for (const auto& location : locations) {
            if (httpRequest.getUri().find(location.first) == 0 && location.first.size() > longestMatch) {
                matchedLocation = location.first;
                longestMatch = location.first.size();
            }
        }

        const auto& locationConfig = locations.at(matchedLocation);
        std::string resolvedPath = resolvePath(httpRequest.getUri(), serverBlocks[i], locationConfig);
        httpRequest.setFilePath(resolvedPath);

        // Debugging Information
        std::cout << "========== DEBUG INFORMATION ==========\n";
        std::cout << "[DEBUG] Received request:\n" << request << "\n";
        std::cout << "[DEBUG] Matched server block index: " << httpRequest.getBlockIndex() << "\n";
        std::cout << "[DEBUG] Received root:\n" << httpRequest.getRootDir() << "\n";
        std::cout << "[DEBUG] Matched server block for host: " << httpRequest.getHeader("host") << "\n";
        std::cout << "[DEBUG] URI: " << httpRequest.getUri() << "\n";
        std::cout << "[DEBUG] Matched location: " << matchedLocation << "\n";
        std::cout << "[DEBUG] Resolved path: " << resolvedPath << "\n";

        // Print allowed methods
        if (locationConfig.find("allow_methods") != locationConfig.end()) {
            std::cout << "[DEBUG] Allowed methods for location " << matchedLocation << ": ";
            std::istringstream methods(locationConfig.at("allow_methods"));
            std::string method;
            while (methods >> method) {
                std::cout << method << " ";
            }
            std::cout << "\n";
        }
        std::cout << "=======================================\n";

        // Check Allowed Methods
        if (locationConfig.find("allow_methods") != locationConfig.end()) {
            std::set<std::string> allowedMethods;
            std::istringstream methods(locationConfig.at("allow_methods"));
            std::string method;
            while (methods >> method) {
                allowedMethods.insert(method);
            }

            if (allowedMethods.find(httpRequest.getMethod()) == allowedMethods.end()) {
                std::cerr << "[DEBUG] Method not allowed: " << httpRequest.getMethod() << " for location " << matchedLocation << "\n";
                HttpResponse response(httpRequest, 405, "Method Not Allowed");
                response.sendResponse(clientSock);
                close(clientSock);
                return;
            }
        }

        // Pass the request to HTTP response handler
        HttpResponse response(httpRequest);
        response.sendResponse(clientSock);

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception while handling client: " << e.what() << "\n";
    }

    close(clientSock);
}



int Server::matchServerBlock(const HttpRequest& httpRequest) {
    std::string host = httpRequest.getHeader("host");
    std::cout << "[DEBUG] Matching server block for host: " << host << "\n";

    for (int i = 0; i < int(serverBlocks.size()); i++) {
        if (serverBlocks[i].directive_pairs.find("server_name") != serverBlocks[i].directive_pairs.end()) {
            if (serverBlocks[i].directive_pairs.at("server_name") == host) {
                std::cout << "[DEBUG] Matched server block for host: " << host << "\n";
                return i;
            }
        }
    }

    std::cout << "[DEBUG] No matching server block found.\n";
    return -1;
}

// std::string Server::resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig) {
//     std::string rootDir = locationConfig.count("root") ? 
//                           locationConfig.at("root") : 
//                           (block.directive_pairs.count("root") ? block.directive_pairs.at("root") : "www");

//     std::string strippedUri = uri;
//     if (locationConfig.count("prefix")) {
//         std::string prefix = locationConfig.at("prefix");
//         if (uri.find(prefix) == 0) {
//             strippedUri = uri.substr(prefix.length());
//         }
//     }

//     std::string path = rootDir + strippedUri;
//     if (path.find("..") != std::string::npos) {
//         throw std::runtime_error("[ERROR] Invalid path: Directory traversal attempt");
//     }

//     std::cerr << "[DEBUG] Resolved path for URI " << uri << ": " << path << "\n";
//     return path;
// }


std::string Server::resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig) {
    // Step 1: Determine the root directory
    std::string rootDir;

    // Check for location-specific root
    if (locationConfig.find("root") != locationConfig.end()) {
        rootDir = locationConfig.at("root");
    }
    // Check for server block root
    else if (block.directive_pairs.find("root") != block.directive_pairs.end()) {
        rootDir = block.directive_pairs.at("root");
    }
    // No root defined
    else {
        throw std::runtime_error("[ERROR] No root defined for the request."); // Return 404 later
    }

    // Step 2: Adjust the URI based on prefix
    std::string strippedUri = uri;
    if (locationConfig.find("prefix") != locationConfig.end()) {
        std::string prefix = locationConfig.at("prefix");
        if (uri.find(prefix) == 0) { // If the URI starts with the prefix
            strippedUri = uri.substr(prefix.length()); // Remove the prefix from the URI
        }
    }

    // Step 3: Construct the final path
    std::string path = rootDir + strippedUri;

    // Step 4: Security check to prevent directory traversal
    if (path.find("..") != std::string::npos)
    {
        throw std::runtime_error("[ERROR] Invalid path: Directory traversal attempt");
    }

    // Debugging information
    std::cerr << "[DEBUG] Resolved path for URI " << uri << ": " << path << "\n";

    return path;
}
