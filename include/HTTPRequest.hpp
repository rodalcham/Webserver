#pragma once

#include "Webserv.hpp"
#include <string>
#include <map>

// Class to represent an HTTP Request
class HttpRequest 
{

private:
	std::string method;                           // HTTP method (e.g., GET, POST)
	std::string uri;                             // Requested URI
	std::string httpVersion;                     // HTTP version (e.g., HTTP/1.1)
	std::map<std::string, std::string> headers;  // Headers as key-value pairs
	std::string body;                            // Request body (e.g., for POST)
	std::string rootDir;
	std::string filePath;

public:

	HttpRequest(const std::string& method,
				const std::string& uri,
				const std::string& httpVersion,
				const std::map<std::string, std::string>& headers,
				const std::string& body);
	~HttpRequest();

	std::string getMethod() const;
	std::string getUri() const;
	std::string getHttpVersion() const;
	std::string getBody() const;
	std::string getHeader(const std::string& key) const;
	std::string getRootDir() const;
	std::string getFilePath () const;

	void	setRootDir (const std::string& rootDir);
	void	setFilePath (const std::string& filePath);

	
	void debug() const;
	// Helper function to get a header value in a case-insensitive manner
	std::string getHeaders(const std::string& headerName) const;
};

// Function to parse an HTTP request string into an HttpRequest object
HttpRequest parseHttpRequest(const std::string& request);
 std::map<std::string, std::string> parseBody(const std::string &body);
