/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 12:13:49 by mbankhar          #+#    #+#             */
/*   Updated: 2024/11/29 10:28:23 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"

// Helper function to parse the HTTP method from the request line
HttpMethod parseHttpMethod(const std::string& methodStr) {
    if (methodStr == "GET") return HttpMethod::GET;
    if (methodStr == "POST") return HttpMethod::POST;
    if (methodStr == "PUT") return HttpMethod::PUT;
    if (methodStr == "DELETE") return HttpMethod::DELETE;
    return HttpMethod::UNKNOWN;
}

// Helper function to parse headers from the request
void parseHeaders(HttpRequest& httpRequest, std::istringstream& requestStream) {
    std::string line;

    while (std::getline(requestStream, line) && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 2); // Skip ": "

            // Normalize header name to lowercase for case-insensitivity
            std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

            httpRequest.headers[headerName] = headerValue;
        }
    }
}

// Main function to parse the HTTP request
HttpRequest parseHttpRequest(const std::string& request) {
    HttpRequest httpRequest;

    std::istringstream requestStream(request);

    // Parse the start line
    std::string startLine;
    std::getline(requestStream, startLine);

    std::istringstream startLineStream(startLine);
    std::string methodStr;
    startLineStream >> methodStr >> httpRequest.uri >> httpRequest.httpVersion;

    // Parse method
    httpRequest.method = parseHttpMethod(methodStr);

    // Parse headers
    parseHeaders(httpRequest, requestStream);

    // Parse body (remaining content after headers)
    std::getline(requestStream, httpRequest.body, '\0');

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
#include "HTTPRequest.hpp"

// Convert HttpMethod enum to a string
std::string HttpRequest::methodToString(HttpMethod method) const {
    switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}
