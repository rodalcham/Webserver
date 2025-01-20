#pragma once

#include "Server.hpp"
#include "HTTPRequest.hpp"
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
#include <deque>
#include <filesystem>

#define SOCKET_BACKLOG_MAX 10

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

#ifndef DEBUG
# define DEBUG 0
#endif

using std::string;
using std::cout;

void	debug(string message);