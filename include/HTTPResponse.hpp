#pragma once

#include "Webserv.hpp"
#include "HTTPRequest.hpp"
#include "ServerBlock.hpp"

#include <fstream>
#include <sstream>

/**
 * HttpResponse
 * @param http_version:
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

	int 			setFilePath(const HttpRequest& request);
	std::string		resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig);
	void			setStatusCode(const int& status_code_no, HttpRequest request);
	void			setBody(bool is_first_try, HttpRequest request);
	void			setHeaders(const int& status_code_no, const HttpRequest& request);
	void			setErrorFilePath(const int& error_code_no, HttpRequest request);

	std::string		getHeaderList();
	std::string		returnResponse();
	void			debug();
};
