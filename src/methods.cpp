#include "../include/Server.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/cgi.hpp"


void Server::handleGet(int clientSock, HttpRequest& httpRequest) {
    // Check if URI corresponds to CGI
    if (isCGIRequest(httpRequest.getUri())) {
        handleCGI(clientSock, httpRequest);
        return;
    }

    // Validate and resolve file path
    std::string filePath = httpRequest.getFilePath();
    char realPath[PATH_MAX];
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        std::cerr << "Invalid path or file not found: " << filePath << std::endl;
        return;
    }

    filePath = std::string(realPath);

    // If the resolved filePath matches the root directory, append index.html
    if (filePath == httpRequest.getRootDir()) {
        filePath += "/index.html";
        httpRequest.setFilePath(filePath);
    }

    // Open the file and check if it's accessible
    std::ifstream file(httpRequest.getFilePath());
    if (!file.good()) {
        std::cerr << "File not found: " << httpRequest.getFilePath() << std::endl;
        return;
    }

    try {
        // Read the file content
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        // Determine the content type based on the file extension
        std::string contentType = getMimeType(httpRequest.getFilePath());

        // Send the response
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << content.size() << "\r\n";
        response << "\r\n";
        response << content;

        std::string responseStr = response.str();
        write(clientSock, responseStr.c_str(), responseStr.size());
    } catch (const std::exception& e) {
        std::cerr << "Error handling GET request: " << e.what() << std::endl;
        return;
    }
}



void Server::handleDelete(int clientSock, HttpRequest& httpRequest) {
	if (2) {
		(void)clientSock; // Suppress unused-variable warnings
	}

	try {
		std::string filePath = resolvePath(httpRequest.getUri());

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
	if (isCGIRequest(httpRequest.getUri())) {
		handleCGI(clientSock, httpRequest);
		return;
	}

	auto expectHeader = httpRequest.getHeader("expect");
	if (!expectHeader.empty()) {
		std::string expectValue = expectHeader;
		std::transform(expectValue.begin(), expectValue.end(), expectValue.begin(), ::tolower);
		if (expectValue == "100-continue") {
			std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
			write(clientSock, continueResponse.c_str(), continueResponse.size());
		}
	}
}
