/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   methods.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 12:17:13 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 12:56:54 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/cgi.hpp"


void Server::handleGet(int clientSock, HttpRequest& httpRequest) {
    if (isCGIRequest(httpRequest.uri)) {
        handleCGI(clientSock, httpRequest);
        return;
    }

    std::string rootDir = serverBlock.directive_pairs["root"];
    std::string filePath = resolvePath(rootDir + httpRequest.uri);

    if (filePath == rootDir) filePath += "/index.html";

    std::ifstream file(filePath);
    if (!file.good()) {
        return;
    }

    try {
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        std::string contentType = getMimeType(filePath);
    } catch (const std::exception& e) {
        return;
    }
}

void Server::handleDelete(int clientSock, HttpRequest& httpRequest) {
if (2) {
    (void)clientSock; // Explicitly mark clientSock as used to suppress unused-variable warnings
}

    try {
        std::string filePath = resolvePath(httpRequest.uri);

        std::ifstream file(filePath);
        if (!file.good()) {
        } else {
            file.close();
            if (remove(filePath.c_str()) != 0) {
                throw std::runtime_error("Failed to delete file");
            }
        }
    } catch (const std::exception& e) {
    }
}

void Server::handlePost(int clientSock, HttpRequest& httpRequest) {
    if (isCGIRequest(httpRequest.uri)) {
        handleCGI(clientSock, httpRequest);
        return;
    }

    auto expectHeader = httpRequest.headers.find("expect");
    if (expectHeader != httpRequest.headers.end()) {
        std::string expectValue = expectHeader->second;
        std::transform(expectValue.begin(), expectValue.end(), expectValue.begin(), ::tolower);
        if (expectValue == "100-continue") {
            std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
            write(clientSock, continueResponse.c_str(), continueResponse.size());
        }
    }
}
