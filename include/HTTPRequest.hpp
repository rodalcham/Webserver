/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gstronge <gstronge@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:51:51 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 19:13:20 by gstronge         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
class HttpRequest 
{

private:
	HttpMethod method;                           // HTTP method (e.g., GET, POST)
	std::string uri;                             // Requested URI
	std::string httpVersion;                     // HTTP version (e.g., HTTP/1.1)
	std::map<std::string, std::string> headers;  // Headers as key-value pairs
	std::string body;                            // Request body (e.g., for POST)

	// Helper function to convert HttpMethod to string
	std::string methodToString(HttpMethod method) const;

public:

	HttpRequest(HttpMethod method,
				const std::string& uri,
				const std::string& httpVersion,
				const std::map<std::string, std::string>& headers,
				const std::string& body);
	~HttpRequest();

	HttpMethod get_method() const;
	std::string get_uri() const;
	std::string get_httpVersion() const;
	std::string get_body() const;
	std::string get_header(const std::string& key) const;

	
	void debug() const;
	// Helper function to get a header value in a case-insensitive manner
	std::string getHeader(const std::string& headerName) const;
};

// Function to parse an HTTP request string into an HttpRequest object
HttpRequest parseHttpRequest(const std::string &request);
 std::map<std::string, std::string> parseBody(const std::string &body);
