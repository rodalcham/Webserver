#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"

void Server::handleGet(int clientSock, HttpRequest& httpRequest) {
    if (isCGIRequest(httpRequest.uri)) {
        handleCGI(clientSock, httpRequest);
        return;
    }

    std::string filePath = resolvePath(httpRequest.uri);
    if (filePath == ROOT_DIR) filePath += "index.html";

    // Check if file exists
    std::ifstream file(filePath);
    if (!file.good()) {
        // Handle file not found
        return;
    }

    try {
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        std::string contentType = getMimeType(filePath);
        // Process and send content
    } catch (const std::exception& e) {
        // Handle errors
        return;
    }
}

void Server::handleDelete(int clientSock, HttpRequest& httpRequest) {
    try {
        std::string filePath = resolvePath(httpRequest.uri);

        // Check if file exists before attempting to delete
        std::ifstream file(filePath);
        if (!file.good()) {
            // sendResponse(clientSock, "404 Not Found", 404, "text/plain");
        } else {
            file.close();
            if (remove(filePath.c_str()) != 0) {
                throw std::runtime_error("Failed to delete file");
            }
            // sendResponse(clientSock, "Resource deleted", 200, "text/plain");
        }
    } catch (const std::exception& e) {
        // sendResponse(clientSock, "500 Internal Server Error", 500, "text/plain");
    }
}
void Server::handlePost(int clientSock, HttpRequest& httpRequest) {
    // Handle "Expect: 100-continue" header
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

    // Check for Content-Length header
    auto contentLengthIt = httpRequest.headers.find("content-length");
    if (contentLengthIt == httpRequest.headers.end()) {
        return; // No Content-Length header; stop further processing
    }
    size_t contentLength = std::stoul(contentLengthIt->second);

    // Check for Content-Type header
    auto contentTypeIt = httpRequest.headers.find("content-type");
    if (contentTypeIt == httpRequest.headers.end()) {
        return; // No Content-Type header; stop further processing
    }

    // Read the body if necessary
    size_t bodyAlreadyRead = httpRequest.body.size();
    if (bodyAlreadyRead < contentLength) {
        std::string requestBody = httpRequest.body; // Copy any already read body content
        size_t totalBytesRead = bodyAlreadyRead;

        while (totalBytesRead < contentLength) {
            char buffer[4096];
            ssize_t bytesRead = read(clientSock, buffer, std::min(sizeof(buffer), contentLength - totalBytesRead));
            if (bytesRead < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(1000); // Sleep for 1 millisecond
                    continue;
                } else if (errno == EINTR) {
                    continue;
                } else {
                    return; // Error reading; stop further processing
                }
            } else if (bytesRead == 0) {
                return; // Incomplete body; stop further processing
            }
            requestBody.append(buffer, bytesRead);
            totalBytesRead += bytesRead;
        }
        httpRequest.body = requestBody; // Update the request body with the full content
    }

    // Process the body based on Content-Type
    std::string contentType = contentTypeIt->second;
    contentType.erase(0, contentType.find_first_not_of(" \t\r\n"));
    contentType.erase(contentType.find_last_not_of(" \t\r\n") + 1);
    std::transform(contentType.begin(), contentType.end(), contentType.begin(), ::tolower);

    if (contentType.find("multipart/form-data") != std::string::npos) {
        // Handle multipart/form-data
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            return; // Boundary missing; stop further processing
        }

        std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        boundary.erase(boundary.find_last_not_of(" \t\r\n") + 1);

        size_t headerStart = httpRequest.body.find(boundary) + boundary.length() + 2;
        size_t headerEnd = httpRequest.body.find("\r\n\r\n", headerStart);
        if (headerEnd == std::string::npos) {
            return; // Invalid multipart headers; stop further processing
        }

        std::string headerPart = httpRequest.body.substr(headerStart, headerEnd - headerStart);

        // Extract filename
        std::string filename = "default_file";
        size_t filenamePos = headerPart.find("filename=\"");
        if (filenamePos != std::string::npos) {
            size_t filenameStart = filenamePos + 10;
            size_t filenameEnd = headerPart.find("\"", filenameStart);
            if (filenameEnd != std::string::npos) {
                filename = headerPart.substr(filenameStart, filenameEnd - filenameStart);
            }
        }

        // Extract file content
        size_t fileContentStart = headerEnd + 4;
        size_t boundaryPosInBody = httpRequest.body.find(boundary, fileContentStart);
        if (boundaryPosInBody == std::string::npos) {
            return; // Invalid multipart form data; stop further processing
        }
        size_t fileContentEnd = boundaryPosInBody - 2;

        std::string fileContent = httpRequest.body.substr(fileContentStart, fileContentEnd - fileContentStart);

        // Save the file
        std::string filePath = "www/uploads/" + filename;
        std::ofstream outFile(filePath, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write(fileContent.c_str(), fileContent.size());
            outFile.close();
        }
    } else if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
        // Handle form data
        std::map<std::string, std::string> formData = parseBody(httpRequest.body);

        // Save form data to a file
        std::string dataFilePath = "www/data/form_data.txt";
        std::ofstream dataFile(dataFilePath, std::ios::app);
        if (dataFile.is_open()) {
            for (const auto& pair : formData) {
                dataFile << pair.first << ": " << pair.second << "\n";
            }
            dataFile << "--------------------------\n";
            dataFile.close();
        }
    }
}
