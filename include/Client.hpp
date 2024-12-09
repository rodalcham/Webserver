/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 15:39:23 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/09 15:06:57 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

/**
 * A class containing client info
 * 
 * @param
 */
class Client
{

	private :

	int					clientSock;
	std::deque<string>	requests;
	std::deque<string>	responses;
	bool				is_sending = false;
	bool				is_receiving = false;

	public : 

	Client(int clientSock);

	int		&getSocket();
	string	&getRequest();
	string	&getResponse();
	void	popResponse();
	void	popRequest();
	bool	&isSending();
	bool	&isReceiving();

	void	parseRequest(char *buffer, size_t bytesRead);
};
