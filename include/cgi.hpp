#pragma once

#include "Webserv.hpp"
#include "HTTPRequest.hpp"


bool isCGIRequest(const std::string& uri);
std::map<std::string, std::string> buildCGIEnvironment(const HttpRequest& httpRequest, const std::string& scriptPath);
std::string unchunkBody(const std::string& body);
void captureCGIOutput(int pipeFd, std::string& output);
void handleCGI(int clientSock, const HttpRequest& httpRequest);


