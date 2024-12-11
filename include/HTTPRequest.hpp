#pragma once

#include "Webserv.hpp"
#include <string>
#include <map>


// Class to represent an HTTP Request
class HttpRequest 
{

private:
	std::string							_method;                           // HTTP method (e.g., GET, POST)
	std::string							_uri;                             // Requested URI
	std::string							_http_version;                     // HTTP version (e.g., HTTP/1.1)
	std::map<std::string, std::string>	_headers;						  // Headers as key-value pairs
	std::string							_body;                            // Request body (e.g., for POST)
	ServerBlock&						_request_block;

public:

	HttpRequest(const std::string& request);
	~HttpRequest();

	std::string getHeader(const std::string& key) const;
	std::string getMethod() const;
	std::string getUri() const;
	std::string getHttpVersion() const;
	std::string	getBody() const;
	
	// Helper function to get a header value in a case-insensitive manner
	std::string							getHeaders(const std::string& headerName) const;
	std::map<std::string, std::string>	parseHeaders(std::istringstream& requestStream);
	std::string							parseHttpMethod(const std::string& methodStr);
	void								debug() const;
};

// Function to parse an HTTP request string into an HttpRequest object
// std::string parseBody(const std::string &body);
