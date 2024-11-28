/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 12:13:49 by mbankhar          #+#    #+#             */
/*   Updated: 2024/11/28 15:42:16 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"

HttpMethod parseHttpMethod(const std::string &methodStr) {
    if (methodStr == "GET") return HttpMethod::GET;
    if (methodStr == "POST") return HttpMethod::POST;
    if (methodStr == "PUT") return HttpMethod::PUT;
    if (methodStr == "DELETE") return HttpMethod::DELETE;
    return HttpMethod::UNKNOWN;
}

HttpRequest parseHttpRequest(const std::string &request) {
    HttpRequest httpRequest;

    std::istringstream requestStream(request);
    std::string startLine;
    std::getline(requestStream, startLine);

    std::istringstream startLineStream(startLine);
    std::string methodStr;
    startLineStream >> methodStr >> httpRequest.uri >> httpRequest.httpVersion;

    httpRequest.method = parseHttpMethod(methodStr);
    return httpRequest;
}
