/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gstronge <gstronge@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/09 11:34:06 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/12 14:53:44 by gstronge         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client() : clientSock(-1)
{

}

Client::Client(int clientSock) : clientSock(clientSock)
{

}

int		&Client::getSocket()
{
	return (this->clientSock);
}

string	&Client::getRequest()
{
	return (this->requests.front());
}

string	&Client::getResponse()
{
	return (this->responses.front());
}

void	Client::popResponse()
{
	this->responses.pop_front();
}

void	Client::popRequest()
{
	this->requests.pop_front();
}

bool	&Client::isSending()
{
	return (this->is_sending);
}

bool	&Client::isReceiving()
{
	return (this->is_receiving);
}

size_t	Client::parseRequest(char *buffer, size_t bytesRead)
{
	size_t	count = 0;
	static const string delimiter = "\r\n\r\n"; // HTTP request delimiter
	size_t i = 0;
	string	request;
	while (i < bytesRead)
	{
		string	temp(buffer + i, bytesRead - i); // wasteful, maybe declare outside?
		size_t	end = temp.find(delimiter);
		if (end != string::npos)
			request = temp.substr(0, end + delimiter.size());
		else
			request = temp.substr(0, bytesRead - i);
		if (this->is_sending)
			this->requests.back() += request;
		else
			this->requests.push_back(request);
		if (end != string::npos)
		{
			this->is_sending = false;
			i += end + delimiter.size();
			count++;
			//post event, maybe return a counter to handle posting in server::recv
		}
		else
			this->is_sending = true;
	}
	return (count);
}
	//Extract a part of the buffer until a /r/n/r/n or the end into a string

	//if isSending is true, add that part into the last request in the queue
	//else make a new request at the back and add it there

	//if the request is not complete, set isSending to true
	//else create an event to proccess the completed request

	//repeat until the buffer is empty
