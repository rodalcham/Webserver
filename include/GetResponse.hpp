#pragma once

#include "Webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

#include <fstream>
#include <sstream>

/**
 * GetResponse
 * 
 * A class for ? (
 * Seriously guys what ara all this classes for)
 * 
 */
class GetResponse : public HttpResponse
{
public:
	GetResponse();
	GetResponse(HttpRequest& request);
	~GetResponse();

	void		makeHeaderList(HttpRequest& request);
	void		parseBody();
	// void 		errorResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");
};
