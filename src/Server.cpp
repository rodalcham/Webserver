/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:51 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/06 12:55:07 by rchavez          ###   ########.fr       */
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

class log;

extern std::atomic<bool> keepRunning;

Server::Server(ServerBlock& serverBlock) : serverBlock(serverBlock) {
	int port = std::stoi(serverBlock.directive_pairs["listen"]);
	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock < 0) throw std::runtime_error("Socket creation failed");

	int flags = fcntl(serverSock, F_GETFL, 0);
	if (flags < 0 || fcntl(serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Failed to set non-blocking mode");

	int opt = 1;
	if (setsockopt(this->serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Failed to set non-blocking mode");

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
	std::cout << "Server Destroyed\n";
}

void Server::run() {
	while (keepRunning) {
		struct kevent eventList[1024];
		int eventCount = kevent(kq, nullptr, 0, eventList, 1024, nullptr);

		if (eventCount < 0) {
			if (errno == EINTR) continue;
			throw std::runtime_error("kevent() failed");
		}

		for (int i = 0; i < eventCount; ++i) {
			int eventSock = eventList[i].ident;

			if (eventSock == serverSock) {
				acceptClient();
			} else {
				handleClient(eventSock);
			}
		}
	}
}

void Server::acceptClient() {
	int clientSock = accept(serverSock, nullptr, nullptr);
	if (clientSock < 0) throw std::runtime_error("Failed to accept new client");

	int flags = fcntl(clientSock, F_GETFL, 0);
	if (flags < 0 || fcntl(clientSock, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(clientSock);
		throw std::runtime_error("Failed to set non-blocking mode");
	}
		
	struct kevent event;
	EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
	{
		close(clientSock);
		throw std::runtime_error("Failed to add client socket to kqueue");
	}

	debug("Acepted client: " + std::to_string(clientSock));
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

		std::string rootDir = serverBlock.directive_pairs["root"];
		std::string resolvedPath = resolvePath(rootDir + httpRequest.get_uri());
		if (!std::ifstream(resolvedPath).good()) {
			std::cerr << "404 Not Found: " << resolvedPath << std::endl;
			return;
		}
		debug("Received from client " + std::to_string(clientSock) + ":\n" + request);
		handleGet(clientSock, httpRequest);
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
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
