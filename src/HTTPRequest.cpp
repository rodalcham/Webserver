#include "../include/HTTPRequest.hpp"
#include "../include/cgi.hpp"
#include "../include/Webserv.hpp"

HttpRequest::HttpRequest(HttpMethod method,
						const std::string& uri,
						const std::string& httpVersion,
						const std::map<std::string, std::string>& headers,
						const std::string& body)
	: method(method), uri(uri), httpVersion(httpVersion), headers(headers), body(body) {}

// Destructor
HttpRequest::~HttpRequest() {
	// Clean up resources if needed
}

HttpMethod parseHttpMethod(const std::string& methodStr) {
    if (methodStr == "GET") return HttpMethod::GET;
    if (methodStr == "POST") return HttpMethod::POST;
    if (methodStr == "DELETE") return HttpMethod::DELETE;

    // Log unsupported method for debugging
    std::cerr << "Unsupported HTTP method: " << methodStr << std::endl;
    throw std::runtime_error("Unsupported HTTP method: " + methodStr);
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


// Updated parseHeaders function to store headers in a case-insensitive manner
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

            // Trim leading and trailing whitespace from headerName
            headerName.erase(0, headerName.find_first_not_of(" \t\r\n"));
            headerName.erase(headerName.find_last_not_of(" \t\r\n") + 1);

            // Trim leading and trailing whitespace from headerValue
            headerValue.erase(0, headerValue.find_first_not_of(" \t\r\n"));
            headerValue.erase(headerValue.find_last_not_of(" \t\r\n") + 1);

            // Normalize header name to lowercase for case-insensitivity
            std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);

            headers[headerName] = headerValue;
        }
    }

    return headers;
}

HttpRequest parseHttpRequest(const std::string& request) {
    // Split the request into request line, headers, and body
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request: Missing headers or body");
    }

    std::string headerPart = request.substr(0, headerEnd);
    std::string bodyPart = request.substr(headerEnd + 4); // Body starts after \r\n\r\n

    // Parse the request line
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
    std::string methodStr = requestLine.substr(0, methodEnd);
    HttpMethod method = parseHttpMethod(methodStr);

    size_t uriEnd = requestLine.find(' ', methodEnd + 1);
    if (uriEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request line: Missing URI");
    }
    std::string uri = requestLine.substr(methodEnd + 1, uriEnd - methodEnd - 1);
    std::string httpVersion = requestLine.substr(uriEnd + 1);

    // Parse headers
    std::map<std::string, std::string> headers = parseHeaders(requestStream);

    // Decode chunked encoding if applicable
    if (headers.find("transfer-encoding") != headers.end() &&
        headers["transfer-encoding"] == "chunked") {
        bodyPart = unchunkBody(bodyPart);
    }

    // Initialize HttpRequest
    HttpRequest httpRequest(method, uri, httpVersion, headers, bodyPart);

    // Set rootDir and filePath in HttpRequest
    // httpRequest.setRootDir(serverBlock.directive_pairs["root"]);
    // httpRequest.setFilePath(httpRequest.getRootDir() + uri);

    return httpRequest;
}




std::map<std::string, std::string> parseBody(const std::string &body) {
    std::map<std::string, std::string> keyValueMap;
    size_t start = 0, end = 0;

    // Split body by '&' to extract key-value pairs
    while ((end = body.find('&', start)) != std::string::npos) {
        std::string pair = body.substr(start, end - start);
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            keyValueMap[key] = value; // Add key-value pair to map
        }
        start = end + 1;
    }

    // Handle the last pair (or the only pair if no '&' was found)
    std::string pair = body.substr(start);
    size_t equalPos = pair.find('=');
    if (equalPos != std::string::npos) {
        std::string key = pair.substr(0, equalPos);
        std::string value = pair.substr(equalPos + 1);
        keyValueMap[key] = value;
    }

    return keyValueMap;
}

HttpMethod HttpRequest::getMethod() const
{
	return method;
}

std::string HttpRequest::getUri() const
{
	return uri;
}

std::string HttpRequest::getHttpVersion() const
{
	return httpVersion;
}

std::string HttpRequest::getBody() const
{
	return body;
}

std::string HttpRequest::getHeaders(const std::string& key) const
{
	auto it = headers.find(key);
	if (it != headers.end()) {
		return it->second;
	}
	return "";
}

// Get the root directory
std::string HttpRequest::getRootDir() const {
    return rootDir;
}

// Get the file path
std::string HttpRequest::getFilePath() const {
    return filePath;
}

// Set the root directory
void HttpRequest::setRootDir(const std::string& newRootDir) {
    rootDir = newRootDir;
}

// Set the file path
void HttpRequest::setFilePath(const std::string& newFilePath) {
    filePath = newFilePath;
}

std::string HttpRequest::methodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}

// Print the details of the HTTP request
void HttpRequest::debug() const {
    std::cout << "Received HTTP Request:\n";
    std::cout << "Header Connection: " << getHeaders("connection") << "\n\n";
    std::cout << "Header accept: " << getHeaders("accept") << "\n\n";
    // std::cout << "Method: " << get_method() << "\n";
    std::cout << "URI: " << getUri() << "\n";
    std::cout << "HTTP Version: " << getHttpVersion() << "\n";
    std::cout << "Body: " << getBody() << "\n\n";
}
