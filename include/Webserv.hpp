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

#define SOCKET_BACKLOG_MAX 5
#define PORT 8080

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

extern std::atomic<bool> keepRunning;

using std::string;
using std::cout;
