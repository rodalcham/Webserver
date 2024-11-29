/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 11:35:33 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/29 13:39:16 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <sys/event.h>
#include <unistd.h>

class Server {
public:
    Server();
    ~Server();
    void run();

private:
    int serverSock;
    int kq;

    void acceptClient();
    void handleClient(int clientSock);
    void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

    std::string readFile(const std::string& filePath); // Function to read static files
};

#endif // SERVER_HPP
