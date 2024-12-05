/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:31 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 14:20:13 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp" 
#include "HTTPRequest.hpp"
#include "ServerBlock.hpp"


class HttpRequest; // Forward declaration

/**
 * A Class representing the server, used to create a socket and listen to inncoming connections or requests.
 * 
 * @param serverSock An Integer containing the file descriptor for the server socket.
 * @param kq An integer representing the KQueue.
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
    ServerBlock serverBlock;
    int serverSock;
    int kq;

    void acceptClient();
    void handleClient(int clientSock);
    // void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

    std::string readFile(const std::string& filePath); // Function to read static files
    std::string getMimeType(const std::string& filePath);
};


