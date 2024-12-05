/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:31 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 10:29:33 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <sys/event.h>
#include <unistd.h>
#include "HTTPRequest.hpp"
#define ROOT_DIR "www/" // Adjust the directory path as per your project setup


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
    int serverSock;
    int kq;

    void acceptClient();
    void handleClient(int clientSock);
    void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

    std::string readFile(const std::string& filePath); // Function to read static files
    std::string getMimeType(const std::string& filePath);
};

