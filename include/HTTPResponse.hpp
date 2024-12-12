#pragma once

#include "Webserv.hpp"
#include "HTTPRequest.hpp"

#include <fstream>
#include <sstream>

/**
 * HttpResponse
 * 
 * A class to  store a response before its ready to send
 * 
 * @param httpVersion The version number
 * @param status_code The retunr code for the response
 * @param headers A map of all the headers contained in the response
 * @param chunking_required ?
 * @param body The body of the response
 * @param file_path ?
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
