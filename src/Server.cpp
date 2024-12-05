/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:51 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 10:29:53 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"
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
            handleGet(clientSock, httpRequest);
        } else if (httpRequest.method == HttpMethod::POST) {
            handlePost(clientSock, httpRequest);
        } else if (httpRequest.method == HttpMethod::DELETE) {
            handleDelete(clientSock, httpRequest);
        } else {
            sendResponse(clientSock, "501 Not Implemented", 501, "text/plain");
        }
    } catch (const std::exception& e) {
        sendResponse(clientSock, "400 Bad Request: " + std::string(e.what()), 400, "text/plain");
    }

    close(clientSock);
}
// void Server::sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType) {
//     std::string statusLine;
//     if (statusCode == 200) statusLine = "HTTP/1.1 200 OK\r\n";
//     else if (statusCode == 201) statusLine = "HTTP/1.1 201 Created\r\n";
//     else if (statusCode == 400) statusLine = "HTTP/1.1 400 Bad Request\r\n";
//     else if (statusCode == 404) statusLine = "HTTP/1.1 404 Not Found\r\n";
//     else if (statusCode == 501) statusLine = "HTTP/1.1 501 Not Implemented\r\n";

//     std::string response = statusLine +
//                            "Content-Type: " + contentType + "\r\n" +
//                            "Content-Length: " + std::to_string(body.size()) + "\r\n" +
//                            "\r\n" + body;

//     write(clientSock, response.c_str(), response.size());
// }
void Server::sendResponse(int clientSock, const std::string& content, int statusCode, const std::string& contentType) {
    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 " << statusCode << " OK\r\n";
    responseStream << "Content-Type: " << contentType << "\r\n";
    responseStream << "Content-Length: " << content.size() << "\r\n";
    responseStream << "\r\n";
    responseStream << content;

    std::string response = responseStream.str();

    size_t totalBytesSent = 0;
    while (totalBytesSent < response.size()) {
        ssize_t bytesSent = send(clientSock, response.c_str() + totalBytesSent, response.size() - totalBytesSent, 0);
        if (bytesSent < 0) {
            perror("Error sending response");
            break;
        }
        totalBytesSent += bytesSent;
    }
}


std::string Server::readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("File not found");

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}
#include <stdexcept>

std::string Server::resolvePath(const std::string& uri) {
    std::string path = ROOT_DIR + uri;
    if (path.find("..") != std::string::npos) throw std::runtime_error("Invalid path");
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

    return "application/octet-stream";  // Default MIME type
}