/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:51:51 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 12:25:26 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm> // For std::transform

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

	// Helper function to convert HttpMethod to string
	std::string methodToString(HttpMethod method) const;

	public:

	HttpRequest();

	~HttpRequest();

	HttpMethod method;                           // HTTP method (e.g., GET, POST)
	std::string uri;                             // Requested URI
	std::string httpVersion;                     // HTTP version (e.g., HTTP/1.1)
	std::map<std::string, std::string> headers;  // Headers as key-value pairs
	std::string body;                            // Request body (e.g., for POST)

	void debugPrint() const;
	// Helper function to get a header value in a case-insensitive manner
	std::string getHeader(const std::string& headerName) const;
};

// Function to parse an HTTP request string into an HttpRequest object
HttpRequest parseHttpRequest(const std::string &request);

	std::string parseName(const std::string &body);
	std::string parseContent(const std::string &body);
