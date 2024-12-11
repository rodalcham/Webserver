/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez@student.42heilbronn.de <rchavez    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 15:39:23 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/10 17:31:40 by rchavez@stu      ###   ########.fr       */
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

	size_t	parseRequest(char *buffer, size_t bytesRead);
};
