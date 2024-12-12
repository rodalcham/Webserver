#pragma once

#include "Webserv.hpp"
#include <string>
#include <map>

// Enum for supported HTTP methods
enum class HttpMethod
{
	GET,
	POST,
	PUT,
	DELETE,
	UNKNOWN // Add UNKNOWN for unsupported or invalid methods
};

// Class to represent an HTTP Request
/**
 * HttpRequest:
 * A class to store a parsed http request before its processed
 * 
 * @param method The method being requested
 * @param uri The requested uri
 * @param httpVersion The http version
 * @param headers A map containing all of the headers of the request
 * @param body The body of the request
 * @param rootDir ?
 * @param filePath ?
 */
class HttpRequest 
{

private:
	HttpMethod method;                           // HTTP method (e.g., GET, POST)
	std::string uri;                             // Requested URI
	std::string httpVersion;                     // HTTP version (e.g., HTTP/1.1)
	std::map<std::string, std::string> headers;  // Headers as key-value pairs
	std::string body;                            // Request body (e.g., for POST)
	std::string rootDir;
	std::string filePath;

public:

	HttpRequest(HttpMethod method,
				const std::string& uri,
				const std::string& httpVersion,
				const std::map<std::string, std::string>& headers,
				const std::string& body);
	~HttpRequest();

	HttpMethod getMethod() const;
	std::string getUri() const;
	std::string getHttpVersion() const;
	std::string getBody() const;
	std::string getHeader(const std::string& key) const;
	std::string getRootDir() const;
	std::string getFilePath () const;

	static std::string methodToString(HttpMethod method);
	void	setRootDir (const std::string& rootDir);
	void	setFilePath (const std::string& filePath);

	
	void debug() const;
	// Helper function to get a header value in a case-insensitive manner
	std::string getHeaders(const std::string& headerName) const;
};

// Function to parse an HTTP request string into an HttpRequest object
HttpRequest parseHttpRequest(const std::string& request);
 std::map<std::string, std::string> parseBody(const std::string &body);
