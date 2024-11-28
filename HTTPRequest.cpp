/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 12:13:49 by mbankhar          #+#    #+#             */
/*   Updated: 2024/11/28 12:23:39 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"

HttpRequest parseHttpRequest(const std::string &request) {
    HttpRequest httpRequest;
    std::istringstream stream(request);

    // Parse the start-line
    std::string startLine;
    std::getline(stream, startLine);
    std::istringstream startLineStream(startLine);
    startLineStream >> httpRequest.method >> httpRequest.uri >> httpRequest.httpVersion;

    // Parse the headers
    std::string line;
    while (std::getline(stream, line) && line != "\r") {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string headerName = line.substr(0, colon);
            std::string headerValue = line.substr(colon + 1);
            // Trim whitespace
            headerName.erase(headerName.find_last_not_of(" \t\r") + 1);
            headerValue.erase(0, headerValue.find_first_not_of(" \t\r"));
            httpRequest.headers[headerName] = headerValue;
        }
    }

    // Parse the body (if any)
    std::getline(stream, httpRequest.body, '\0');
    return httpRequest;
}
