#pragma once

#include "Webserv.hpp"
#include "HTTPRequest.hpp"
#include <map>
#include <fstream>
#include <sstream>
#include <string>

class HttpRequest; // Forward declaration

/**
 * HttpResponse
 * @param httpVersion:
 */

class HttpResponse
{
protected:
	std::string							httpVersion;
	std::string							status_code;
	std::map<std::string, std::string> headers;
	bool								chunking_required;
	std::string							body;
	std::string							file_path;

public:
	HttpResponse();
	HttpResponse(HttpRequest& request);
	HttpResponse(HttpRequest& request, int statusCode, const std::string& statusMessage); // <-- Add this line
	~HttpResponse();

	std::string	getHeaderList();
	void		sendResponse(int clientSock);
	void 		setBody(const std::string& bodyContent);
	void 		setHeader(const std::string& key, const std::string& value);


	// void 		errorResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

	void		debug();
};
