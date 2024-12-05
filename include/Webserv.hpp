/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:37 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 12:19:07 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <map>
#include <set>
#include <atomic>
#include <iostream>
#include <fstream>

#define SOCKET_BACKLOG_MAX 10
#define PORT 8080

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

#ifndef DEBUG
# define DEBUG 0
#endif

extern std::atomic<bool> keepRunning;

using std::string;
using std::cout;

/*Custom functions*/
uint16_t	ft_htons(uint16_t port);
void		debug(string message);

