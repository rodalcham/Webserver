/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 12:13:49 by mbankhar          #+#    #+#             */
/*   Updated: 2024/11/29 09:48:38 by mbankhar         ###   ########.fr       */
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
