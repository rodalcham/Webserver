#pragma once

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "ServerBlock.hpp"

#include <fstream>
#include <sstream>

/**
 * HttpResponse
 * @param httpVersion:
 */

class HttpResponse
{
protected:
	std::string							_http_version;
	std::string							_status_code;
	std::map<std::string, std::string>	_headers;
	bool								_chunking_required;
	std::string							_body;
	std::string							_file_path;
	const std::map<int, std::string>	_error_status_codes = {
	{200, "200 OK"}, {201, "201 Created"}, {400, "400 Bad Request"}, 
	{401, "401 Unauthorized"}, {403, "403 Forbidden"}, {404, "404 Not Found"}, 
	{405, "405 Method Not Allowed"}, {413, "413 Payload Too Large"}, 
	{415, "415 Unsupported Media Type"}, {500, "500 Internal Server Error"}, 
	{501, "501 Not Implemented"}, {502, "502 Bad Gateway"}, 
	{503, "503 Service Unavailable"}, {504, "504 Gateway Timeout"}};

public:
	HttpResponse();
	HttpResponse(const HttpRequest& request);
	~HttpResponse();

	std::string	getHeaderList();
	void		parseBody();
	void		sendResponse(int clientSock);

	int 		setFilePath(const HttpRequest& request);
	void		setStatusCode(const int& status_code_no);
	void 		setErrorResponse(const int& error_code);
	void		setErrorHeaders(const int& error_code);
	void		setErrorBody(const int& error_code);

	void		debug();
};
