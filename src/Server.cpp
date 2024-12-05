/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:51 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 14:11:33 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

#define PORT 8080
// #define ROOT_DIR "www/" 

extern std::atomic<bool> keepRunning;

// Server::Server() : serverBlock(*(new ServerBlock())) {
//     // Initialize with default ServerBlock if no arguments are passed
// }

Server::Server(ServerBlock& serverBlock) : serverBlock(serverBlock) {
    int port = std::stoi(serverBlock.directive_pairs["listen"]);
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) throw std::runtime_error("Socket creation failed");

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		throw std::runtime_error("Bind failed");

    if (listen(serverSock, 5) < 0)
        throw std::runtime_error("Listen failed");

	kq = kqueue();
	if (kq < 0) throw std::runtime_error("kqueue creation failed");

	struct kevent event;
	EV_SET(&event, serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
		throw std::runtime_error("Failed to add server socket to kqueue");
}

Server::~Server() {
	close(serverSock);
	close(kq);
	debug("Server Destroyed");
}

void Server::run() {
	while (keepRunning) {
		struct kevent eventList[1024];
		int eventCount = kevent(this->kq, nullptr, 0, eventList, 1024, nullptr);

		if (eventCount < 0) {
			if (errno == EINTR) continue; // Handle signal interruption
			throw std::runtime_error("kevent() failed");
		}

		for (int i = 0; i < eventCount; ++i) {
			int eventSock = eventList[i].ident;

			if (eventSock == this->serverSock) {
				this->acceptClient();
			} else {
				this->handleClient(eventSock);
			}
		}
	}
}

void Server::acceptClient() {
	int clientSock = accept(serverSock, nullptr, nullptr);
	if (clientSock < 0) throw std::runtime_error("Failed to accept new client");

	struct kevent event;
	EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
		throw std::runtime_error("Failed to add client socket to kqueue");
	debug("Client accepted: " + std::to_string(clientSock));
}

void Server::handleClient(int clientSock) {
    char buffer[1024];
    ssize_t bytes = read(clientSock, buffer, sizeof(buffer));
    if (bytes < 0) {
        std::cerr << "Failed to read from client\n";
        close(clientSock);
        return;
    }
    if (bytes == 0) {
        std::cerr << "Client disconnected\n";
        close(clientSock);
        return;
    }

    std::string request(buffer, bytes);
    std::cout << "Received Request:\n" << request << std::endl;

    try {
        HttpRequest httpRequest = parseHttpRequest(request);

        std::string rootDir = serverBlock.directive_pairs["root"];
        std::string errorPage404 = serverBlock.error_pages["404"];
        std::string errorPage400 = serverBlock.error_pages["400"];

        if (httpRequest.method == HttpMethod::GET) {
            std::string resolvedPath = resolvePath(rootDir + httpRequest.uri);
            if (!std::ifstream(resolvedPath).good()) {
                if (!errorPage404.empty()) {
                    std::string errorContent = readFile(errorPage404);
                } else {
                    std::cerr << "404 Not Found\n";
                }
                return;
            }
            handleGet(clientSock, httpRequest);
        } else if (httpRequest.method == HttpMethod::POST) {
            handlePost(clientSock, httpRequest);
        } else if (httpRequest.method == HttpMethod::DELETE) {
            handleDelete(clientSock, httpRequest);
        } else {
            std::cerr << "501 Not Implemented\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
        // if (!errorPage400.empty()) {
        //     std::string errorContent = readFile(errorPage400);
        // } else {
        //     std::cerr << "400 Bad Request\n";
        // }
    }

    close(clientSock);
}

std::string Server::readFile(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) throw std::runtime_error("File not found");

	std::ostringstream content;
	content << file.rdbuf();
	return content.str();
}

std::string Server::resolvePath(const std::string& uri) {
    // Root directory from configuration or default macro
    std::string rootDir = serverBlock.directive_pairs.count("root") ? 
                          serverBlock.directive_pairs["root"] : "www";

    // Combine root directory with the requested URI
    std::string path = /*rootDir +*/ uri;

    // Sanitize and validate the path to prevent directory traversal
    if (path.find("..") != std::string::npos) {
        throw std::runtime_error("Invalid path: Directory traversal attempt");
    }

    // Debug logging to print the resolved path
    std::cout << "[DEBUG] Resolved path: " << path << std::endl;

    return path;
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
