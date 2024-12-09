#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/cgi.hpp"
#include <iostream>
#include <fstream>

// GET Handler
void Server::handleGet(int clientSock, HttpRequest& httpRequest) {
		int i = matchServerBlock(httpRequest);
		if (i < 0)
			std::cout << "SOME ERROR\n";
        std::string filePath = resolvePath(httpRequest.getUri(), serverBlocks[i]);
    httpRequest.setFilePath(filePath);

    char realPath[PATH_MAX];
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        // Invalid path or file not found
        HttpResponse response(httpRequest, 404, "Not Found");
        response.sendResponse(clientSock);
        return;
    }

    filePath = std::string(realPath);
    httpRequest.setFilePath(filePath);

    // If filePath == rootDir exactly, append /index.html
    if (filePath == httpRequest.getRootDir()) {
        filePath += "/index.html";
        httpRequest.setFilePath(filePath);
    }

    // Construct and send a 200 OK response that reads and returns the file
    HttpResponse response(httpRequest);
    response.sendResponse(clientSock);
}

// DELETE Handler
void Server::handleDelete(int clientSock, HttpRequest& httpRequest) {
    try {
		int i = matchServerBlock(httpRequest);
		if (i < 0)
			std::cout << "SOME ERROR\n"; // need to fix this so that it doesn't carry on in the code and give a segfault
        std::string filePath = resolvePath(httpRequest.getUri(), serverBlocks[i]);

        std::ifstream file(filePath);
        if (!file.good()) {
            // 404 Not Found
            HttpResponse response(httpRequest, 404, "Not Found");
            response.sendResponse(clientSock);
        } else {
            file.close();
            if (remove(filePath.c_str()) != 0) {
                // Failed to delete -> 500
                HttpResponse response(httpRequest, 500, "Internal Server Error");
                response.sendResponse(clientSock);
            } else {
                // Successfully deleted -> 200
                HttpResponse response(httpRequest, 200, "OK");
                response.sendResponse(clientSock);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling DELETE request: " << e.what() << std::endl;
        HttpResponse response(httpRequest, 500, "Internal Server Error");
        response.sendResponse(clientSock);
    }
}

// POST Handler
void Server::handlePost(int clientSock, HttpRequest& httpRequest) {
    auto expectHeader = httpRequest.getHeader("expect");
    if (!expectHeader.empty()) {
        std::string expectValue = expectHeader;
        std::transform(expectValue.begin(), expectValue.end(), expectValue.begin(), ::tolower);
        if (expectValue == "100-continue") {
            std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
            write(clientSock, continueResponse.c_str(), continueResponse.size());
        }
    }

    // For now, just send 200 OK
    HttpResponse response(httpRequest, 200, "OK");
    response.sendResponse(clientSock);
}
