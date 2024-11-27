/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 11:16:39 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/27 16:02:40 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{
	cout << GREEN << "Connecting server";

	this->serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSock < 0)
		throw std::runtime_error("Socket Creation Failed");

	int	flags = fcntl(this->serverSock, F_GETFL, 0);
    if (flags < 0)
        throw std::runtime_error("Failed to Get Socket Flags");
    if  (fcntl(this->serverSock, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to Set Socket Flags");

	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_port = /*htons not allowed*/ htons(PORT);
	this->serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(this->serverSock, (struct sockaddr *)&this->serverAddr, sizeof(this->serverAddr)) < 0)
		throw std::runtime_error("Socket Bind Failed");
	if (listen(this->serverSock, SOCKET_BACKLOG_MAX) < 0)
		throw std::runtime_error("Socket Listen Failed");
	cout << ".";

	this->kq = kqueue();
	if (this->kq < 0)
		throw std::runtime_error("Kqueue Creation Failed");
	cout << ".";
	
	struct	kevent	event;
	EV_SET(&event, this->serverSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
		throw std::runtime_error("Failed to Add Socket to Kqueue");
	cout << ".\nConnected.\n" << RESET;

	this->events[this->serverSock] = event;
}

Server::~Server()
{
	close(this->serverSock);
	close(this->kq);
	cout << GREEN << "Server Disconected.\n";
}

void	Server::run()
{
	while (I_AM_THE_BEST)
	{
		struct kevent eventList[1024];
        
        // Wait for events on the kqueue
        int eventCount = kevent(this->kq, nullptr, 0, eventList, 1024, nullptr);
        if (eventCount < 0)
            throw std::runtime_error("kevent() failed");

        for (int i = 0; i < eventCount; ++i) {
            int eventSock = eventList[i].ident;

            // Check if the server socket has an event (new connection)
            if (eventSock == this->serverSock) {
                try {
                    this->acceptClient(); // Accept the new client
                } catch (const std::exception &e) {
                    std::cerr << "Error accepting client: " << e.what() << std::endl;
                }
            }
            // Check if a client socket has data or a disconnection event
            else {
                try {
                    // Handle the event for this client socket
                    this->handleClient(eventSock);
                } catch (const std::exception &e) {
                    std::cerr << "Error handling client " << eventSock << ": " << e.what() << std::endl;
                }
            }
        }
	}
}

void	Server::acceptClient()
{
	int	clientSock = accept(this->serverSock, nullptr, nullptr);
	if (clientSock < 0)
		throw std::runtime_error("Failed to Accept New Client");

	struct kevent	event;
	EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
		throw std::runtime_error("Failed to Add Client to Kqueue");

	this->events[clientSock] = event;
}

void	Server::handleClient(int clientSock)
{
	char	buffer[1024];
	ssize_t	bytes = read(clientSock, buffer, sizeof(buffer));

	if (bytes < 0)
		throw std::runtime_error("Failed to Read from Client");
	if (bytes == 0)
	{
		struct kevent	event;
		EV_SET(&event, clientSock, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
		if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
			throw std::runtime_error("Failed to Remove Client from Kqueue");
		close(clientSock);
		this->events.erase(clientSock);
		return;
	}

	/*TEMPORARY*/
	cout << GREEN << "RECEIVED : " << RESET << string(buffer, bytes) << "\n";
	const string body = "<html><body><h1>Hello, World!</h1></body></html>";
	const string response = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                        "Connection: close\r\n\r\n" + body;
	write(clientSock, response.c_str(), response.length());
	/*TEMPORARY*/
}