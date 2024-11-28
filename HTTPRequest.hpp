/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 11:37:46 by mbankhar          #+#    #+#             */
/*   Updated: 2024/11/28 15:40:50 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
};

class HttpRequest {
public:
    HttpMethod method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
};


HttpRequest parseHttpRequest(const std::string &request);

#endif // HTTPREQUEST_HPP
