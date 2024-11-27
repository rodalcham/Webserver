/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 11:35:33 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/27 15:06:03 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

class Server
{

private:

	int								serverSock;
	sockaddr_in						serverAddr;
	int								kq;
	std::map<int, struct kevent>	events;

public:

	Server();
	~Server();

	void	run();
	void	acceptClient();
	void	handleClient(int clientSock);

};
