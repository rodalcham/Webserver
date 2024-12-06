/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:31 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/06 15:39:13 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp" 
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "ServerBlock.hpp"


class HttpRequest; // Forward declaration
class HttpResponse; // Forward declaration
class Client;

/**
 * A Class representing the server, used to create a socket and listen to inncoming connections or requests.
 * 
 * @param serverSock An Integer containing the file descriptor for the server socket.
 * @param kq An integer representing the KQueue.
 * @param clients A map containing all the clients, indexedy their socket fd.
 */
class Server {
public:
	// Server();
	~Server();
	Server(ServerBlock& serverBlock);
	void run();
	void handleGet(int clientSock, HttpRequest& httpRequest);
	void handlePost(int clientSock, HttpRequest& httpRequest);
	void handleDelete(int clientSock, HttpRequest& httpRequest);
	std::string resolvePath(const std::string& uri);

private:
	ServerBlock 			serverBlock;
	int 					serverSock;
	int 					kq;
	std::map<int, Client>	clients

	void acceptClient();
	void handleClient(int clientSock);
	// void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

	std::string readFile(const std::string& filePath); // Function to read static files
	std::string getMimeType(const std::string& filePath);
	
	void		msg_send(int clientSock, HttpResponse &msg);
	HttpRequest	&msg_receive(int clientSock);
};


