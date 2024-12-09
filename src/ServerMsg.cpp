/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMsg.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 12:37:14 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/09 15:35:37 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include <errno.h>

void	Server::enable_write_listen(int	clientSock)
{
	struct kevent enable;
	EV_SET(&enable, clientSock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(this->kq, &enable, 1, nullptr, 0, nullptr) < 0)
	{
		close(clientSock);
		this->clients.erase(clientSock);
		throw std::runtime_error("Failed to listen for write readiness");
	}
}
void	Server::disable_write_listen(int	clientSock)
{
	struct kevent disable;
	EV_SET(&disable, clientSock, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
	if (kevent(this->kq, &disable, 1, nullptr, 0, nullptr) < 0)
	{
		close(clientSock);
		this->clients.erase(clientSock);
		throw std::runtime_error("Failed disable listen for write readiness");
	}
}

void	Server::postEvent(int clientSock, int mode)
{
	struct kevent	event;
	int				ident;


	ident = (clientSock * 10) + mode;
	EV_SET(&event, ident, EVFILT_USER, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0) {
		perror("Failed to post custom event to kqueue");
	}
}

void		Server::msg_send(Client &client, int mode)
{
	size_t	bytes;

	if (!mode && client.isReceiving())
	{
		this->postEvent(client.getSocket(), 2);
		return;
	}

	string	*msg =&client.getResponse();

	while (!msg->empty())
	{
		bytes = send(client.getSocket(), msg->data(), msg->size(), 0);
		if	(bytes > 0)
		{
			msg->erase(0, bytes);
			if (msg->empty())
			{
				client.popResponse();
				if (mode)
				{
					disable_write_listen(client.getSocket());
					client.isReceiving() = false;
				}
			}
		}
		else if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			if (!mode)
			{
				client.isReceiving() = true;
				enable_write_listen(client.getSocket());
			}
			break;
		}
		else
		{
			if (mode)
				disable_write_listen(client.getSocket());
			close(client.getSocket());
			this->clients.erase(client.getSocket());
			throw std::runtime_error("Failed to send to client socket " + std::to_string(client.getSocket()) + ": " + strerror(errno));
		}
	}
}

void	Server::msg_receive(Client &client, int mode)
{
	const size_t bufferSize = 4096;
	char	buffer[bufferSize];
	size_t	bytes;

	bytes = recv(client.getSocket(), buffer, bufferSize, 0);

	if (bytes > 0)
	{
		client.parseRequest(&buffer[0], bytes);
		if (bytes == bufferSize)
			this->postEvent(client.getSocket(), 0);
	}
	else if (bytes == 0 && !mode)
	{
		close(client.getSocket());
		this->clients.erase(client.getSocket());
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;//Not sure
		close(client.getSocket());
    	this->clients.erase(client.getSocket());
    	throw std::runtime_error("Failed to receive from client socket " + std::to_string(client.getSocket()) + ": " + strerror(errno));
	}
}