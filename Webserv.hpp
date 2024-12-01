#pragma once

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
#include "Server.hpp"
#include "HTTPRequest.hpp"
#include <fstream>
#include <iostream>
#include "cgi.hpp"


extern bool keepRunning;
#define ROOT_DIR "www"

#define SOCKET_BACKLOG_MAX 5
#define PORT 8080

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

using std::string;
using std::cout;
