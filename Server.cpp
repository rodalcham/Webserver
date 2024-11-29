/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 11:16:39 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/29 09:48:55 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "HTTPRequest.hpp"


Server::Server() {
    cout << GREEN << "Connecting server";

    // Create a non-blocking socket
    this->serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (this->serverSock < 0)
        throw std::runtime_error("Socket Creation Failed");

    int flags = fcntl(this->serverSock, F_GETFL, 0);
    if (flags < 0)
        throw std::runtime_error("Failed to Get Socket Flags");
    if (fcntl(this->serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to Set Socket Flags");

    // Configure the server address
    this->serverAddr.sin_family = AF_INET;
    this->serverAddr.sin_port = htons(PORT);
    this->serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(this->serverSock, (struct sockaddr *)&this->serverAddr, sizeof(this->serverAddr)) < 0)
        throw std::runtime_error("Socket Bind Failed");
    if (listen(this->serverSock, SOCKET_BACKLOG_MAX) < 0)
        throw std::runtime_error("Socket Listen Failed");
    cout << ".";

    // Create the kqueue instance
    this->kq = kqueue();
    if (this->kq < 0)
        throw std::runtime_error("Kqueue Creation Failed");
    cout << ".";

    // Add the server socket to the kqueue for monitoring
    struct kevent event;
    EV_SET(&event, this->serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
        throw std::runtime_error("Failed to Add Socket to Kqueue");
    cout << ".\nConnected.\n" << RESET;

    this->events[this->serverSock] = event;
}

Server::~Server() {
    close(this->serverSock);
    close(this->kq);
    cout << GREEN << "Server Disconnected.\n" << RESET;
}

void Server::run() {
    while (keepRunning) {
        struct kevent eventList[1024];
        int eventCount = kevent(this->kq, nullptr, 0, eventList, 1024, nullptr);

        if (eventCount < 0) {
            if (errno == EINTR) {
                continue; // Handle signal interruption gracefully
            }
            throw std::runtime_error("kevent() failed");
        }

        for (int i = 0; i < eventCount; ++i) {
            int eventSock = eventList[i].ident;

            if (eventSock == this->serverSock) {
                try {
                    this->acceptClient();
                } catch (const std::exception &e) {
                    std::cerr << "Error accepting client: " << e.what() << std::endl;
                }
            } else {
                try {
                    this->handleClient(eventSock);
                } catch (const std::exception &e) {
                    std::cerr << "Error handling client " << eventSock << ": " << e.what() << std::endl;
                }
            }
        }
    }
}

void Server::acceptClient() {
    int clientSock = accept(this->serverSock, nullptr, nullptr);
    if (clientSock < 0)
        throw std::runtime_error("Failed to Accept New Client");

    struct kevent event;
    EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
        throw std::runtime_error("Failed to Add Client to Kqueue");

    this->events[clientSock] = event;
}

void Server::handleClient(int clientSock) {
    char buffer[1024];
    ssize_t bytes = read(clientSock, buffer, sizeof(buffer));
    if (bytes < 0) {
        throw std::runtime_error("Failed to Read from Client");
    }
    if (bytes == 0) {
        // Client disconnected
        struct kevent event;
        EV_SET(&event, clientSock, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
            throw std::runtime_error("Failed to Remove Client from Kqueue");
        close(clientSock);
        this->events.erase(clientSock);
        return;
    }

    // Parse the request
    std::string request(buffer, bytes);
    HttpRequest httpRequest = parseHttpRequest(request);


    // Send a basic response (temporary)
    const std::string body = "<html><body><h1>Hello, World!</h1></body></html>";
    const std::string response = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/html\r\n"
                                 "Content-Length: " + std::to_string(body.length()) + "\r\n"
                                 "Connection: close\r\n\r\n" + body;
    write(clientSock, response.c_str(), response.length());
}

