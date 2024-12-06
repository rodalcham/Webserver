#include "../include/HTTPRequest.hpp"
#include "../include/cgi.hpp"
#include "../include/Webserv.hpp"

std::string unchunkBody(const std::string& body) {
    std::string result;
    size_t pos = 0;

    while (pos < body.size()) {
        // Find the end of the chunk size line
        size_t chunkSizeEnd = body.find("\r\n", pos);
        if (chunkSizeEnd == std::string::npos) break;

        // Parse the chunk size
        int chunkSize = std::stoi(body.substr(pos, chunkSizeEnd - pos), nullptr, 16);
        if (chunkSize == 0) break; // End of chunked body

        pos = chunkSizeEnd + 2; // Move past the chunk size and \r\n
        result += body.substr(pos, chunkSize);
        pos += chunkSize + 2; // Move past the chunk and its trailing \r\n
    }

    return result;
}

HttpRequest::HttpRequest(const std::string& method,
                         const std::string& uri,
                         const std::string& httpVersion,
                         const std::map<std::string, std::string>& headers,
                         const std::string& body)
    : method(method), uri(uri), httpVersion(httpVersion), headers(headers), body(body) {}

HttpRequest::~HttpRequest() {}

// Getters
std::string HttpRequest::getMethod() const {
    return method;
}

std::string HttpRequest::getUri() const {
    return uri;
}

std::string HttpRequest::getHttpVersion() const {
    return httpVersion;
}

std::string HttpRequest::getBody() const {
    return body;
}

std::string HttpRequest::getHeader(const std::string& key) const {
    std::string normalizedKey = key;
    std::transform(normalizedKey.begin(), normalizedKey.end(), normalizedKey.begin(), ::tolower);
    auto it = headers.find(normalizedKey);
    if (it != headers.end()) {
        return it->second;
    }
    return ""; // Return empty string if header not found
}

std::string HttpRequest::getRootDir() const {
    return rootDir;
}

std::string HttpRequest::getFilePath() const {
    return filePath;
}

// Setters
void HttpRequest::setRootDir(const std::string& rootDir) {
    this->rootDir = rootDir;
}

void HttpRequest::setFilePath(const std::string& filePath) {
    this->filePath = filePath;
}

void HttpRequest::debug() const {
    std::cout << "HTTP Request Debug:\n"
              << "Method: " << method << "\n"
              << "URI: " << uri << "\n"
              << "HTTP Version: " << httpVersion << "\n"
              << "Headers:\n";
    for (const auto& header : headers) {
        std::cout << "  " << header.first << ": " << header.second << "\n";
    }
    std::cout << "Body: " << body << "\n"
              << "Root Directory: " << rootDir << "\n"
              << "File Path: " << filePath << "\n";
}

std::map<std::string, std::string> parseHeaders(std::istringstream& requestStream) {
    std::map<std::string, std::string> headers;
    std::string line;

    while (std::getline(requestStream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back(); // Remove trailing \r
        }
        if (line.empty()) {
            break; // End of headers
        }

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1); // Skip ':'

            headerName.erase(0, headerName.find_first_not_of(" \t"));
            headerName.erase(headerName.find_last_not_of(" \t") + 1);

            headerValue.erase(0, headerValue.find_first_not_of(" \t"));
            headerValue.erase(headerValue.find_last_not_of(" \t") + 1);

            std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

            headers[headerName] = headerValue;
        }
    }

    return headers;
}

HttpRequest parseHttpRequest(const std::string& request) {
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request: Missing headers or body");
    }

    std::string headerPart = request.substr(0, headerEnd);
    std::string bodyPart = request.substr(headerEnd + 4);

    std::istringstream requestStream(headerPart);
    std::string requestLine;
    std::getline(requestStream, requestLine);

    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back(); // Remove trailing \r
    }

    size_t methodEnd = requestLine.find(' ');
    if (methodEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request line: Missing method");
    }
    std::string method = requestLine.substr(0, methodEnd);

    size_t uriEnd = requestLine.find(' ', methodEnd + 1);
    if (uriEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request line: Missing URI");
    }
    std::string uri = requestLine.substr(methodEnd + 1, uriEnd - methodEnd - 1);
    std::string httpVersion = requestLine.substr(uriEnd + 1);

    std::map<std::string, std::string> headers = parseHeaders(requestStream);

    if (headers.find("transfer-encoding") != headers.end() &&
        headers["transfer-encoding"] == "chunked") {
        bodyPart = unchunkBody(bodyPart);
    }

    HttpRequest httpRequest(method, uri, httpVersion, headers, bodyPart);

    // Example: Setting rootDir and filePath (could be customized)
    httpRequest.setRootDir("www");
    httpRequest.setFilePath(httpRequest.getRootDir() + uri);

    return httpRequest;
}

std::map<std::string, std::string> parseBody(const std::string& body) {
    std::map<std::string, std::string> keyValueMap;
    size_t start = 0, end = 0;

    while ((end = body.find('&', start)) != std::string::npos) {
        std::string pair = body.substr(start, end - start);
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            keyValueMap[key] = value;
        }
        start = end + 1;
    }

    std::string pair = body.substr(start);
    size_t equalPos = pair.find('=');
    if (equalPos != std::string::npos) {
        std::string key = pair.substr(0, equalPos);
        std::string value = pair.substr(equalPos + 1);
        keyValueMap[key] = value;
    }

    return keyValueMap;
}

std::string HttpRequest::getHeaders(const std::string& headerName) const {
    auto it = headers.find(headerName);
    if (it != headers.end()) {
        return it->second;
    }
    return ""; // Return empty string if the header is not found
}
