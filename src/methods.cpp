#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/cgi.hpp"


void Server::handleGet(int clientSock, HttpRequest& httpRequest) {
    // Match the server block for this request
    ServerBlock& matchedBlock = matchServerBlock(httpRequest);

    // Resolve the path using the matched block
    std::string filePath = resolvePath(httpRequest.getUri(), matchedBlock);
    httpRequest.setFilePath(filePath);

    char realPath[PATH_MAX];
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        std::cerr << "Invalid path or file not found: " << filePath << std::endl;
        // Here you might want to send a 404 response:
        HttpResponse response(httpRequest, 404, "Not Found");
        response.sendResponse(clientSock);
        return;
    }

    filePath = std::string(realPath);
    httpRequest.setFilePath(filePath);

    // If the resolved filePath matches the root directory exactly, try index.html
    if (filePath == httpRequest.getRootDir()) {
        filePath += "/index.html";
        httpRequest.setFilePath(filePath);
    }

    std::ifstream file(httpRequest.getFilePath());
    if (!file.good()) {
        std::cerr << "File not found: " << httpRequest.getFilePath() << std::endl;
        HttpResponse response(httpRequest, 404, "Not Found");
        response.sendResponse(clientSock);
        return;
    }

    // Since HttpResponse presumably reads the file and sets headers, etc.
    // Just create and send it using HttpResponse
    HttpResponse response(httpRequest);
    response.sendResponse(clientSock);
}




void Server::handleDelete(int clientSock, HttpRequest& httpRequest) {
    (void)clientSock; // In case we don't use it yet, but we likely will.

    try {
        ServerBlock& matchedBlock = matchServerBlock(httpRequest);
        std::string filePath = resolvePath(httpRequest.getUri(), matchedBlock);

        std::ifstream file(filePath);
        if (!file.good()) {
            // File doesn't exist, send a 404
            HttpResponse response(httpRequest, 404, "Not Found");
            response.sendResponse(clientSock);
        } else {
            file.close();
            if (remove(filePath.c_str()) != 0) {
                // Failed to delete
                HttpResponse response(httpRequest, 500, "Internal Server Error");
                response.sendResponse(clientSock);
            } else {
                // Successfully deleted
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



void Server::handlePost(int clientSock, HttpRequest& httpRequest) {
    // If you need the correct server block and path:
    // ServerBlock& matchedBlock = matchServerBlock(httpRequest);
    // std::string filePath = resolvePath(httpRequest.getUri(), matchedBlock);
    // httpRequest.setFilePath(filePath);

    auto expectHeader = httpRequest.getHeader("expect");
    if (!expectHeader.empty()) {
        std::string expectValue = expectHeader;
        std::transform(expectValue.begin(), expectValue.end(), expectValue.begin(), ::tolower);
        if (expectValue == "100-continue") {
            std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
            write(clientSock, continueResponse.c_str(), continueResponse.size());
        }
    }

    // Handle post data as needed...

    // For now, just respond with a 200 OK or some other appropriate response
    HttpResponse response(httpRequest, 200, "OK");
    response.sendResponse(clientSock);
}
