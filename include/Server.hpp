/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gstronge <gstronge@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:31 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 13:41:32 by gstronge         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <sys/event.h>
#include <unistd.h>
#include "HTTPRequest.hpp"
#define ROOT_DIR "www/" // Adjust the directory path as per your project setup

/**
 * A Class representing the server, used to create a socket and listen to inncoming connections or requests.
 * 
 * @param serverSock An Integer containing the file descriptor for the server socket.
 * @param kq An integer representing the KQueue.
 */
class Server {
public:
    Server();
    ~Server();
    void run();
    void handleGet(int clientSock, HttpRequest& httpRequest);
    void handlePost(int clientSock, HttpRequest& httpRequest);
    void handleDelete(int clientSock, HttpRequest& httpRequest);
    std::string resolvePath(const std::string& uri);

private:
    int		serverSock;
    int		kq;

    void acceptClient();
    void handleClient(int clientSock);
    void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

    std::string readFile(const std::string& filePath); // Function to read static files
    std::string getMimeType(const std::string& filePath);
};

