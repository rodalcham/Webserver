/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 11:16:39 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/29 13:40:10 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "HTTPRequest.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define ROOT_DIR "www/" // Directory for static files

extern std::atomic<bool> keepRunning;


Server::Server() {
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) throw std::runtime_error("Socket creation failed");

    int flags = fcntl(serverSock, F_GETFL, 0);
    if (flags < 0 || fcntl(serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to set non-blocking mode");

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        throw std::runtime_error("Bind failed");

    if (listen(serverSock, 5) < 0) throw std::runtime_error("Listen failed");

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

    HttpRequest httpRequest = parseHttpRequest(request);

    try {
        if (httpRequest.method == HttpMethod::GET) {
            std::string filePath = ROOT_DIR + httpRequest.uri.substr(1);
            if (filePath == ROOT_DIR) filePath += "index.html";
            std::string content = readFile(filePath);
            sendResponse(clientSock, content, 200, "text/html");
        } else {
            sendResponse(clientSock, "Method Not Allowed", 405, "text/plain");
        }
    } catch (const std::exception &e) {
        sendResponse(clientSock, "Error: " + std::string(e.what()), 400, "text/plain");
    }
    close(clientSock);
}
void Server::sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType) {
    std::string statusLine;
    if (statusCode == 200) statusLine = "HTTP/1.1 200 OK\r\n";
    else if (statusCode == 201) statusLine = "HTTP/1.1 201 Created\r\n";
    else if (statusCode == 400) statusLine = "HTTP/1.1 400 Bad Request\r\n";
    else if (statusCode == 404) statusLine = "HTTP/1.1 404 Not Found\r\n";
    else if (statusCode == 501) statusLine = "HTTP/1.1 501 Not Implemented\r\n";

    std::string response = statusLine +
                           "Content-Type: " + contentType + "\r\n" +
                           "Content-Length: " + std::to_string(body.size()) + "\r\n" +
                           "\r\n" + body;

    write(clientSock, response.c_str(), response.size());
}


std::string Server::readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("File not found");

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}
