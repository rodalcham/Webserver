/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:41 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 17:51:07 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/HTTPRequest.hpp"
#include "../include/cgi.hpp"
#include "../include/Webserv.hpp"


// Constructor
HttpRequest::HttpRequest()
	: method(HttpMethod::UNKNOWN), uri("/"), httpVersion("HTTP/1.1") {
	headers.clear();
	body.clear();
}

// Destructor
HttpRequest::~HttpRequest() {
	// Clean up resources if needed
}

// Helper function to parse the HTTP method from the request line
HttpMethod parseHttpMethod(const std::string& methodStr) {
	if (methodStr == "GET") return HttpMethod::GET;
	if (methodStr == "POST") return HttpMethod::POST;
	if (methodStr == "DELETE") return HttpMethod::DELETE;
	throw std::runtime_error("Unsupported HTTP method: " + methodStr);
}

// Helper function to parse headers from the request
void parseHeaders(HttpRequest& httpRequest, std::istringstream& requestStream) {
	std::string line;

	while (std::getline(requestStream, line)) {
		if (!line.empty() && line.back() == '\r') {
			line.pop_back(); // Remove trailing \r
		}
		if (line.empty()) {
			break; // End of headers
		}
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			std::string headerName = line.substr(0, colonPos);
			std::string headerValue = line.substr(colonPos + 1); // Skip ':'

			// Trim leading and trailing whitespace from headerName
			headerName.erase(0, headerName.find_first_not_of(" \t\r\n"));
			headerName.erase(headerName.find_last_not_of(" \t\r\n") + 1);

			// Trim leading and trailing whitespace from headerValue
			headerValue.erase(0, headerValue.find_first_not_of(" \t\r\n"));
			headerValue.erase(headerValue.find_last_not_of(" \t\r\n") + 1);

			// Normalize header name to lowercase for case-insensitivity
			std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

			httpRequest.headers[headerName] = headerValue;
		}
	}
}

HttpRequest parseHttpRequest(const std::string& request) {
	HttpRequest httpRequest;

	// Split the request into request line, headers, and body
	size_t headerEnd = request.find("\r\n\r\n");
	if (headerEnd == std::string::npos) throw std::runtime_error("Invalid HTTP request");

	std::string headerPart = request.substr(0, headerEnd);
	std::string bodyPart = request.substr(headerEnd + 4); // Body starts after \r\n\r\n

	std::istringstream requestStream(headerPart);

	// Parse request line
	std::string requestLine;
	std::getline(requestStream, requestLine);
	if (!requestLine.empty() && requestLine.back() == '\r') {
		requestLine.pop_back(); // Remove trailing \r
	}
	size_t methodEnd = requestLine.find(' ');
	if (methodEnd == std::string::npos) throw std::runtime_error("Invalid HTTP request line");

	std::string methodStr = requestLine.substr(0, methodEnd);
	httpRequest.method = parseHttpMethod(methodStr);

	size_t uriEnd = requestLine.find(' ', methodEnd + 1);
	if (uriEnd == std::string::npos) throw std::runtime_error("Invalid HTTP request line");

	httpRequest.uri = requestLine.substr(methodEnd + 1, uriEnd - methodEnd - 1);
	httpRequest.httpVersion = requestLine.substr(uriEnd + 1);

	// Parse headers
	parseHeaders(httpRequest, requestStream);

    // Decode chunked encoding if applicable
    if (httpRequest.headers.find("transfer-encoding") != httpRequest.headers.end() &&
        httpRequest.headers["transfer-encoding"] == "chunked") {
        bodyPart = unchunkBody(bodyPart);
    }

    // Set body
    httpRequest.body = bodyPart;

	return httpRequest;
}



// Print the details of the HTTP request
void HttpRequest::debugPrint() const {
	std::cout << "Received HTTP Request:\n";
	std::cout << "Method: " << methodToString(method) << "\n";
	std::cout << "URI: " << uri << "\n";
	std::cout << "HTTP Version: " << httpVersion << "\n";
	std::cout << "Headers:\n";
	for (const auto& header : headers) {
		std::cout << "  " << header.first << ": " << header.second << "\n";
	}
	std::cout << "Body: " << body << "\n\n";
}

// Convert HttpMethod enum to a string, for debug only
std::string HttpRequest::methodToString(HttpMethod method) const {
	switch (method) {
		case HttpMethod::GET: return "GET";
		case HttpMethod::POST: return "POST";
		case HttpMethod::PUT: return "PUT";
		case HttpMethod::DELETE: return "DELETE";
		default: return "UNKNOWN";
	}
}

std::map<std::string, std::string> parseBody(const std::string &body) {
    std::map<std::string, std::string> keyValueMap;
    size_t start = 0, end = 0;

    // Split body by '&' to extract key-value pairs
    while ((end = body.find('&', start)) != std::string::npos) {
        std::string pair = body.substr(start, end - start);
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            keyValueMap[key] = value; // Add key-value pair to map
        }
        start = end + 1;
    }

    // Handle the last pair (or the only pair if no '&' was found)
    std::string pair = body.substr(start);
    size_t equalPos = pair.find('=');
    if (equalPos != std::string::npos) {
        std::string key = pair.substr(0, equalPos);
        std::string value = pair.substr(equalPos + 1);
        keyValueMap[key] = value;
    }

    return keyValueMap;
}

HttpMethod HttpRequest::get_method() const {
	return method; // Return the private member `method`
}

std::string HttpRequest::get_uri() const {
	return uri; // Return the private member `uri`
}

std::string HttpRequest::get_httpVersion() const {
	return httpVersion; // Return the private member `httpVersion`
}

std::string HttpRequest::get_body() const {
	return body; // Return the private member `body`
}

std::string HttpRequest::get_header(const std::string& key) const
{
	auto it = headers.find(key);
	if (it != headers.end()) {
		return it->second; // Return the value if the key exists
	}
	return ""; // Return an empty string if the key is not found
}
