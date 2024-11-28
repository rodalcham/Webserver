/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 11:35:33 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/28 12:26:47 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Webserv.hpp"
#include <map>
#include <sys/event.h>
#include <netinet/in.h>

class Server {
private:
    int serverSock;
    struct sockaddr_in serverAddr;
    int kq;
    std::map<int, struct kevent> events;

public:
    Server();
    ~Server();
    void run();
    void acceptClient();
    void handleClient(int clientSock);
};

#endif // SERVER_HPP
