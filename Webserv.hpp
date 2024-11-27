/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 11:43:39 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/27 16:02:52 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define SOCKET_BACKLOG_MAX 5
#define PORT 8080

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

#define I_AM_THE_BEST true

#ifndef DEBUG
# define DEBUG 0
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <map>

using std::string;
using std::cout;
