/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 11:37:46 by mbankhar          #+#    #+#             */
/*   Updated: 2024/11/28 12:31:04 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>

class HttpRequest {
public:
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;

void debugPrint() const {
    std::cout << "Method: " << method << "\n";
    std::cout << "URI: " << uri << "\n";
    std::cout << "HTTP Version: " << httpVersion << "\n";
    std::cout << "Headers:\n";

    // Use explicit types and traditional loop
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << "\n";
    }

    std::cout << "Body: " << body << "\n";
}

};

HttpRequest parseHttpRequest(const std::string &request);

#endif // HTTPREQUEST_HPP
