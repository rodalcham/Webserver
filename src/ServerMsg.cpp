/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMsg.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 12:37:14 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/06 13:17:12 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include <errno.h>

void		Server::msg_send(int clientSock, HttpResponse &msg)
{
	size_t	data_size;
	size_t	data_sent;
	size_t	bytes;

	string msg = "HEYO";

	data_size = sizeof(msg);
	data_sent = 0;

	struct kevent enable;
	EV_SET(&enable, clientSock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(this->kq, &enable, 1, nullptr, 0, nullptr) < 0)
	{
		close(clientSock);
		throw std::runtime_error("Failed to listen for write readiness");
	}

	while (data_sent < data_size)
	{
		bytes = send(clientSock, , data_size - data_sent, 0);
	}

	struct kevent disable;
	EV_SET(&disable, clientSock, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
	if (kevent(this->kq, &disable, 1, nullptr, 0, nullptr) < 0)
	{
		close(clientSock);
		throw std::runtime_error("Failed disable listen for write readiness");
	}
}

HttpRequest	&msg_receive(int clientSock);