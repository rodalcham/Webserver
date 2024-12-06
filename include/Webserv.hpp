#pragma once

#include "Server.hpp"
#include "HTTPRequest.hpp"
#include "cgi.hpp"
#include "ServerBlock.hpp"
#include <sys/event.h>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <algorithm>
#include <csignal>
#include <atomic>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <fstream>

#define SOCKET_BACKLOG_MAX 10
// #define PORT 8080

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

