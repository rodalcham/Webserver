#pragma once

#include "Webserv.hpp"
#include "HTTPRequest.hpp"

#include <fstream>
#include <sstream>

/**
 * HttpResponse
 * @param httpVersion:
 */

class HttpResponse
{
protected:
	std::string							httpVersion;
	std::string							status_code;
	std::map<std::string, std::string>	headers;
	bool								chunking_required;
	std::string							body;
	std::string							file_path;

public:
	HttpResponse();
	HttpResponse(HttpRequest& request);
	~HttpResponse();

	std::string	getHeaderList();
	void		sendResponse(int clientSock);
	// void 		errorResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

	void		debug();
};
