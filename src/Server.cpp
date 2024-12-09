// #include "../include/Webserv.hpp"
// #include "../include/Server.hpp"
// #include "../include/HTTPRequest.hpp"
// #include "../include/HTTPResponse.hpp"

// #include <iostream>
// #include <sstream>
// #include <fstream>
// #include <stdexcept>
// #include <fcntl.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <errno.h>
// #include <algorithm>

// extern std::atomic<bool> keepRunning;

// Server::Server(const std::vector<ServerBlock>& blocks) : serverBlocks(blocks) {
//     if (serverBlocks.empty()) {
//         throw std::runtime_error("No server blocks provided");
//     }

//     // Create the kqueue
//     kq = kqueue();
//     if (kq < 0) throw std::runtime_error("kqueue creation failed");

//     // For each ServerBlock, create a listening socket
//     for (size_t i = 0; i < serverBlocks.size(); ++i) {
//         const ServerBlock& block = serverBlocks[i];

//         // Ensure the block has a listen directive
//         if (block.directive_pairs.find("listen") == block.directive_pairs.end()) {
//             throw std::runtime_error("No listen directive found in a server block");
//         }

//         int port = std::stoi(block.directive_pairs.at("listen"));
//         int serverSock = socket(AF_INET, SOCK_STREAM, 0);
//         if (serverSock < 0) throw std::runtime_error("Socket creation failed");

//         int flags = fcntl(serverSock, F_GETFL, 0);
//         if (flags < 0 || fcntl(serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
//             throw std::runtime_error("Failed to set non-blocking mode");

//         int opt = 1;
//         if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
//             throw std::runtime_error("Failed to set socket options");

//         sockaddr_in serverAddr{};
//         serverAddr.sin_family = AF_INET;
//         serverAddr.sin_port = htons(port);
//         serverAddr.sin_addr.s_addr = INADDR_ANY;

//         if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
//             throw std::runtime_error("Bind failed");

//         if (listen(serverSock, 5) < 0)
//             throw std::runtime_error("Listen failed");

//         // Add this listening socket to kqueue
//         struct kevent event;
//         EV_SET(&event, serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
//         if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
//             throw std::runtime_error("Failed to add server socket to kqueue");

//         listenSockets.push_back(serverSock);
//         // Map this socket to the current server block
//         socketToBlockMap[serverSock] = (ServerBlock*)&block;
//     }
// }

// Server::~Server() {
//     for (int sock : listenSockets) {
//         close(sock);
//     }
//     close(kq);
//     std::cout << "Server Destroyed\n";
// }

// void Server::run() {
//     while (keepRunning) {
//         struct kevent eventList[1024];
//         int eventCount = kevent(kq, nullptr, 0, eventList, 1024, nullptr);

//         if (eventCount < 0) {
//             if (errno == EINTR) continue; // Handle interrupts
//             throw std::runtime_error("kevent() failed");
//         }

//         for (int i = 0; i < eventCount; ++i) {
//             int eventSock = (int)eventList[i].ident;

//             // Check if it's a listening socket event
//             if (socketToBlockMap.find(eventSock) != socketToBlockMap.end()) {
//                 acceptClient(eventSock);
//             } else {
//                 // It's a client socket event
//                 handleClient(eventSock);
//             }
//         }
//     }
// }

// void Server::acceptClient(int listeningSock) {
//     int clientSock = accept(listeningSock, nullptr, nullptr);
//     if (clientSock < 0) {
//         return;
//     }

//     // Add clientSock to kqueue
//     struct kevent event;
//     EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
//     if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0) {
//         close(clientSock);
//         return;
//     }
// }

// void Server::handleClient(int clientSock) {
//     char buffer[1024];
//     ssize_t bytes = read(clientSock, buffer, sizeof(buffer));
//     if (bytes <= 0) {
//         close(clientSock);
//         return;
//     }

//     std::string request(buffer, bytes);
//     try {
//         HttpRequest httpRequest = parseHttpRequest(request);
//         // Match the ServerBlock based on Host header
//         ServerBlock& matchedBlock = matchServerBlock(httpRequest);

//         httpRequest.setRootDir(matchedBlock.directive_pairs.at("root"));
//         httpRequest.setFilePath(resolvePath(httpRequest.getUri(), matchedBlock));

//         if (!std::ifstream(httpRequest.getFilePath()).good()) {
//             // File not found
//             HttpResponse response(httpRequest, 404, "Not Found");
//             response.sendResponse(clientSock);
//             close(clientSock);
//             return;
//         }

//         if (!matchedBlock.isRequestAllowed(httpRequest)) {
//             // Not allowed
//             HttpResponse response(httpRequest, 403, "Forbidden");
//             response.sendResponse(clientSock);
//             close(clientSock);
//             return;
//         }

//         // Handle methods
//         if (httpRequest.getMethod() == "GET") {
//             handleGet(clientSock, httpRequest);
//         } else if (httpRequest.getMethod() == "POST") {
//             handlePost(clientSock, httpRequest);
//         } else if (httpRequest.getMethod() == "DELETE") {
//             handleDelete(clientSock, httpRequest);
//         } else {
//             // Method not supported
//             HttpResponse response(httpRequest, 405, "Method Not Allowed");
//             response.sendResponse(clientSock);
//         }

//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//     }

//     close(clientSock);
// }

// ServerBlock& Server::matchServerBlock(const HttpRequest& httpRequest) {
//     std::string host = httpRequest.getHeader("host");
//     for (auto& block : serverBlocks) {
//         if (block.directive_pairs.find("server_name") != block.directive_pairs.end()) {
//             if (block.directive_pairs.at("server_name") == host) {
//                 return block;
//             }
//         }
//     }
//     // If no match found, return the first block
//     return serverBlocks[0];
// }

// std::string Server::resolvePath(const std::string& uri, const ServerBlock& block) {
//     std::string rootDir = block.directive_pairs.count("root") ? 
//                           block.directive_pairs.at("root") : "www";

//     std::string path = rootDir + uri;
//     if (path.find("..") != std::string::npos) {
//         throw std::runtime_error("Invalid path: Directory traversal attempt");
//     }
//     std::cout << "[DEBUG] Resolved path: " << path << std::endl;
//     return path;
// }

// std::string Server::readFile(const std::string& filePath) {
//     std::ifstream file(filePath, std::ios::binary);
//     if (!file.is_open()) throw std::runtime_error("File not found");
//     std::ostringstream content;
//     content << file.rdbuf();
//     return content.str();
// }

// std::string Server::getMimeType(const std::string& filePath) {
//     size_t dotPos = filePath.find_last_of('.');
//     if (dotPos == std::string::npos) return "application/octet-stream";
//     std::string extension = filePath.substr(dotPos + 1);
//     if (extension == "html" || extension == "htm") return "text/html";
//     if (extension == "css") return "text/css";
//     if (extension == "js") return "application/javascript";
//     if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
//     if (extension == "png") return "image/png";
//     if (extension == "gif") return "image/gif";
//     if (extension == "txt") return "text/plain";
//     return "application/octet-stream";
// }

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

        // Ensure the block has a listen directive
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

        // Add this listening socket to kqueue
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

        ServerBlock& matchedBlock = matchServerBlock(httpRequest);
        std::cout << "[DEBUG] Matched server block for request.\n";

        httpRequest.setRootDir(matchedBlock.directive_pairs.at("root"));
        httpRequest.setFilePath(resolvePath(httpRequest.getUri(), matchedBlock));

        if (!std::ifstream(httpRequest.getFilePath()).good()) {
            std::cerr << "[DEBUG] File not found: " << httpRequest.getFilePath() << "\n";
            HttpResponse response(httpRequest, 404, "Not Found");
            response.sendResponse(clientSock);
            close(clientSock);
            return;
        }

        if (!matchedBlock.isRequestAllowed(httpRequest)) {
            std::cerr << "[DEBUG] Request not allowed for URI: " << httpRequest.getUri() << "\n";
            HttpResponse response(httpRequest, 403, "Forbidden");
            response.sendResponse(clientSock);
            close(clientSock);
            return;
        }

        std::cout << "[DEBUG] Handling HTTP method: " << httpRequest.getMethod() << "\n";

        if (httpRequest.getMethod() == "GET") {
            handleGet(clientSock, httpRequest);
        } else if (httpRequest.getMethod() == "POST") {
            handlePost(clientSock, httpRequest);
        } else if (httpRequest.getMethod() == "DELETE") {
            handleDelete(clientSock, httpRequest);
        } else {
            std::cerr << "[DEBUG] HTTP method not supported: " << httpRequest.getMethod() << "\n";
            HttpResponse response(httpRequest, 405, "Method Not Allowed");
            response.sendResponse(clientSock);
        }

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception while handling client: " << e.what() << "\n";
    }

    close(clientSock);
}

ServerBlock& Server::matchServerBlock(const HttpRequest& httpRequest) {
    std::string host = httpRequest.getHeader("host");
    std::cout << "[DEBUG] Matching server block for host: " << host << "\n";

    for (auto& block : serverBlocks) {
        if (block.directive_pairs.find("server_name") != block.directive_pairs.end()) {
            if (block.directive_pairs.at("server_name") == host) {
                std::cout << "[DEBUG] Matched server block for host: " << host << "\n";
                return block;
            }
        }
    }

    std::cout << "[DEBUG] No matching server block found. Using default block.\n";
    return serverBlocks[0];
}

// std::string Server::resolvePath(const std::string& uri, const ServerBlock& block) {
//     std::string rootDir = block.directive_pairs.count("root") ? 
//                           block.directive_pairs.at("root") : "www";

//     std::string path = rootDir + uri;
//     if (path.find("..") != std::string::npos) {
//         throw std::runtime_error("Invalid path: Directory traversal attempt");
//     }
//     std::cout << "[DEBUG] Resolved path: " << path << "\n";
//     return path;
// }

std::string Server::resolvePath(const std::string& uri, const ServerBlock& block) {
    std::string rootDir = block.directive_pairs.count("root") ? 
                          block.directive_pairs.at("root") : "www";

    std::string path = rootDir + uri;
    if (uri == "/") {
        path += "/index.html"; // Add default index
    }

    if (path.find("..") != std::string::npos) {
        throw std::runtime_error("[ERROR] Invalid path: Directory traversal attempt");
    }

    std::cerr << "[DEBUG] Resolved path for URI " << uri << ": " << path << "\n";
    return path;
}
