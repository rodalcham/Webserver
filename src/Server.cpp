#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/Webserv.hpp"
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

class log;


extern std::atomic<bool> keepRunning;

// Server::Server(ServerBlock& serverBlock) : serverBlock(serverBlock) {
//     int port = std::stoi(serverBlock.directive_pairs["listen"]);
//     serverSock = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSock < 0) throw std::runtime_error("Socket creation failed");

//     int flags = fcntl(serverSock, F_GETFL, 0);
//     if (flags < 0 || fcntl(serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
//         throw std::runtime_error("Failed to set non-blocking mode");

// 	int opt = 1;
// 	if (setsockopt(this->serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
//         throw std::runtime_error("Failed to set non-blocking mode");

//     sockaddr_in serverAddr{};
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     serverAddr.sin_addr.s_addr = INADDR_ANY;

//     if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
//         throw std::runtime_error("Bind failed");

//     if (listen(serverSock, 5) < 0)
//         throw std::runtime_error("Listen failed");

//     kq = kqueue();
//     if (kq < 0) throw std::runtime_error("kqueue creation failed");

//     struct kevent event;
//     EV_SET(&event, serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
//     if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
//         throw std::runtime_error("Failed to add server socket to kqueue");
// }

// ServerBlock& Server::matchServerBlock(const HttpRequest& httpRequest) {
//     for (auto& block : serverBlocks) {
//         if (block.directive_pairs.at("server_name") == httpRequest.getHeader("host")) {
//             return block;
//         }
//     }
//     return serverBlocks[0]; // Default to the first block if no match
// }

// Server::Server(const std::vector<ServerBlock>& blocks) : serverBlocks(blocks) {
//     if (serverBlocks.empty()) {
//         throw std::runtime_error("No server blocks provided");
//     }

//     const ServerBlock& initialBlock = serverBlocks[0];
//     int port = std::stoi(initialBlock.directive_pairs.at("listen"));

//     serverSock = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSock < 0) throw std::runtime_error("Socket creation failed");

//     int flags = fcntl(serverSock, F_GETFL, 0);
//     if (flags < 0 || fcntl(serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
//         throw std::runtime_error("Failed to set non-blocking mode");

//     int opt = 1;
//     if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
//         throw std::runtime_error("Failed to set socket options");

//     sockaddr_in serverAddr{};
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     serverAddr.sin_addr.s_addr = INADDR_ANY;

//     if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
//         throw std::runtime_error("Bind failed");

//     if (listen(serverSock, 5) < 0)
//         throw std::runtime_error("Listen failed");

//     kq = kqueue();
//     if (kq < 0) throw std::runtime_error("kqueue creation failed");

//     struct kevent event;
//     EV_SET(&event, serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
//     if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
//         throw std::runtime_error("Failed to add server socket to kqueue");
// }

// Server::~Server() {
//     close(serverSock);
//     close(kq);
//     std::cout << "Server Destroyed\n";
// }

// void Server::run() {
//     while (keepRunning) {
//         struct kevent eventList[1024];
//         int eventCount = kevent(kq, nullptr, 0, eventList, 1024, nullptr);

//         if (eventCount < 0) {
//             if (errno == EINTR) continue;
//             throw std::runtime_error("kevent() failed");
//         }

//         for (int i = 0; i < eventCount; ++i) {
//             int eventSock = eventList[i].ident;

//             if (eventSock == serverSock) {
//                 acceptClient();
//             } else {
//                 handleClient(eventSock);
//             }
//         }
//     }
// }


// void Server::acceptClient() {
//     int clientSock = accept(serverSock, nullptr, nullptr);
//     if (clientSock < 0) throw std::runtime_error("Failed to accept new client");

//     struct kevent event;
//     EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
//     if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
//         throw std::runtime_error("Failed to add client socket to kqueue");
// 		debug("Acepted client: " + std::to_string(clientSock));
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

//         httpRequest.setRootDir(serverBlock.directive_pairs["root"]);
//         httpRequest.setFilePath(httpRequest.getRootDir() + httpRequest.getUri());
//         // serverBlock.debugPrint();

//         if (!std::ifstream(httpRequest.getFilePath()).good()) {
//             std::cerr << "404 Not Found: " << httpRequest.getFilePath() << std::endl;
//             return;
//         }
//         debug("Received from client " + std::to_string(clientSock) + ":\n" + request);
//         if (!serverBlock.isRequestAllowed(httpRequest))
//         {
//             std::cerr << "Request denied: " << httpRequest.getUri() << std::endl;
//             return;
//         }
//         handleGet(clientSock, httpRequest);
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//     }
//     close(clientSock);
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

//         // Match the server block for this request
//         ServerBlock& matchedBlock = matchServerBlock(httpRequest);

//         httpRequest.setRootDir(matchedBlock.directive_pairs.at("root"));
//         httpRequest.setFilePath(resolvePath(httpRequest.getUri(), matchedBlock));

//         if (!std::ifstream(httpRequest.getFilePath()).good()) {
//             std::cerr << "404 Not Found: " << httpRequest.getFilePath() << std::endl;
//             return;
//         }

//         if (!matchedBlock.isRequestAllowed(httpRequest)) {
//             std::cerr << "Request denied: " << httpRequest.getUri() << std::endl;
//             return;
//         }

//         handleGet(clientSock, httpRequest);
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//     }
//     close(clientSock);
// }


// std::string Server::readFile(const std::string& filePath) {
// 	std::ifstream file(filePath, std::ios::binary);
// 	if (!file.is_open()) throw std::runtime_error("File not found");

// 	std::ostringstream content;
// 	content << file.rdbuf();
// 	return content.str();
// }

// std::string Server::resolvePath(const std::string& uri) {
//     // Root directory from configuration or default macro
//     std::string rootDir = serverBlock.directive_pairs.count("root") ? 
//                           serverBlock.directive_pairs["root"] : "www";

//     // Combine root directory with the requested URI
//     std::string path = /*rootDir +*/ uri;

//     // Sanitize and validate the path to prevent directory traversal
//     if (path.find("..") != std::string::npos) {
//         throw std::runtime_error("Invalid path: Directory traversal attempt");
//     }

//     // Debug logging to print the resolved path
//     std::cout << "[DEBUG] Resolved path: " << path << std::endl;

//     return path;
// }


// std::string Server::getMimeType(const std::string& filePath) {
// 	size_t dotPos = filePath.find_last_of('.');
// 	if (dotPos == std::string::npos) return "application/octet-stream";

// 	std::string extension = filePath.substr(dotPos + 1);

// 	if (extension == "html" || extension == "htm") return "text/html";
// 	if (extension == "css") return "text/css";
// 	if (extension == "js") return "application/javascript";
// 	if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
// 	if (extension == "png") return "image/png";
// 	if (extension == "gif") return "image/gif";
// 	if (extension == "txt") return "text/plain";

//     return "application/octet-stream";
// }


Server::Server(const std::vector<ServerBlock>& blocks) : serverBlocks(blocks) {
    if (serverBlocks.empty()) {
        throw std::runtime_error("No server blocks provided");
    }

    // Create the kqueue
    kq = kqueue();
    if (kq < 0) throw std::runtime_error("kqueue creation failed");

    // For each ServerBlock, create a listening socket
    for (size_t i = 0; i < serverBlocks.size(); ++i) {
        const ServerBlock& block = serverBlocks[i];

        // Ensure the block has a listen directive
        if (block.directive_pairs.find("listen") == block.directive_pairs.end()) {
            throw std::runtime_error("No listen directive found in a server block");
        }

        int port = std::stoi(block.directive_pairs.at("listen"));
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

        // Add this listening socket to kqueue
        struct kevent event;
        EV_SET(&event, serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
            throw std::runtime_error("Failed to add server socket to kqueue");

        listenSockets.push_back(serverSock);
        // Map this socket to the current server block
        socketToBlockMap[serverSock] = (ServerBlock*)&block;
    }
}

Server::~Server() {
    for (int sock : listenSockets) {
        close(sock);
    }
    close(kq);
    std::cout << "Server Destroyed\n";
}

void Server::run() {
    while (keepRunning) {
        struct kevent eventList[1024];
        int eventCount = kevent(kq, nullptr, 0, eventList, 1024, nullptr);

        if (eventCount < 0) {
            if (errno == EINTR) continue; // Handle interrupts
            throw std::runtime_error("kevent() failed");
        }

        for (int i = 0; i < eventCount; ++i) {
            int eventSock = (int)eventList[i].ident;

            // Check if it's a listening socket event
            if (socketToBlockMap.find(eventSock) != socketToBlockMap.end()) {
                acceptClient(eventSock);
            } else {
                // It's a client socket event
                handleClient(eventSock);
            }
        }
    }
}

void Server::acceptClient(int listeningSock) {
    int clientSock = accept(listeningSock, nullptr, nullptr);
    if (clientSock < 0) {
        return;
    }

    // Add clientSock to kqueue
    struct kevent event;
    EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0) {
        close(clientSock);
        return;
    }
}

void Server::handleClient(int clientSock) {
    char buffer[1024];
    ssize_t bytes = read(clientSock, buffer, sizeof(buffer));
    if (bytes <= 0) {
        close(clientSock);
        return;
    }

    std::string request(buffer, bytes);
    try {
        HttpRequest httpRequest = parseHttpRequest(request);
        // Match the ServerBlock based on Host header
        ServerBlock& matchedBlock = matchServerBlock(httpRequest);

        httpRequest.setRootDir(matchedBlock.directive_pairs.at("root"));
        httpRequest.setFilePath(resolvePath(httpRequest.getUri(), matchedBlock));

        if (!std::ifstream(httpRequest.getFilePath()).good()) {
            std::cerr << "404 Not Found: " << httpRequest.getFilePath() << std::endl;
            // Send a 404 response if needed...
            close(clientSock);
            return;
        }

        if (!matchedBlock.isRequestAllowed(httpRequest)) {
            std::cerr << "Request denied: " << httpRequest.getUri() << std::endl;
            // Send a 403 or similar if needed...
            close(clientSock);
            return;
        }

        // Handle methods (only GET implemented here as example)
        if (httpRequest.getMethod() == "GET") {
            handleGet(clientSock, httpRequest);
        } else if (httpRequest.getMethod() == "POST") {
            handlePost(clientSock, httpRequest);
        } else if (httpRequest.getMethod() == "DELETE") {
            handleDelete(clientSock, httpRequest);
        } else {
            // Method not supported
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    close(clientSock);
}

ServerBlock& Server::matchServerBlock(const HttpRequest& httpRequest) {
    std::string host = httpRequest.getHeader("host");
    for (auto& block : serverBlocks) {
        if (block.directive_pairs.find("server_name") != block.directive_pairs.end()) {
            if (block.directive_pairs.at("server_name") == host) {
                return block;
            }
        }
    }
    // If no match found, return the first block
    return serverBlocks[0];
}

std::string Server::resolvePath(const std::string& uri, const ServerBlock& block) {
    std::string rootDir = block.directive_pairs.count("root") ? 
                          block.directive_pairs.at("root") : "www";

    std::string path = rootDir + uri;
    if (path.find("..") != std::string::npos) {
        throw std::runtime_error("Invalid path: Directory traversal attempt");
    }
    std::cout << "[DEBUG] Resolved path: " << path << std::endl;
    return path;
}

std::string Server::readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("File not found");
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

std::string Server::getMimeType(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) return "application/octet-stream";
    std::string extension = filePath.substr(dotPos + 1);
    if (extension == "html" || extension == "htm") return "text/html";
    if (extension == "css") return "text/css";
    if (extension == "js") return "application/javascript";
    if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
    if (extension == "png") return "image/png";
    if (extension == "gif") return "image/gif";
    if (extension == "txt") return "text/plain";
    return "application/octet-stream";
}
