#include "../include/Webserv.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/ServerBlock.hpp"
#include "../include/cgi.hpp"

HttpRequest::HttpRequest(const std::string& request, const ServerBlock request_server_block)
{
	this->_request_block = request_server_block;
	// Split the request into request line, headers, and body
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
		throw std::runtime_error("Invalid HTTP request: Missing headers or body");

    std::string headerPart = request.substr(0, headerEnd);
    this->_body = request.substr(headerEnd + 4); // Body starts after \r\n\r\n

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
    this->_method = parseHttpMethod(methodStr);

    size_t uriEnd = requestLine.find(' ', methodEnd + 1);
    if (uriEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request line: Missing URI");
    }
    this->_uri = requestLine.substr(methodEnd + 1, uriEnd - methodEnd - 1);
    this->_http_version = requestLine.substr(uriEnd + 1);

    // Parse headers
    this->_headers = parseHeaders(requestStream);

    // Decode chunked encoding if applicable
    if (headers.find("transfer-encoding") != headers.end() &&
        headers["transfer-encoding"] == "chunked") {
        this->_body = unchunkBody(_body);
    }
}
// Destructor
HttpRequest::~HttpRequest()
{
	// Clean up resources if needed
}

std::string HttpRequest::parseHttpMethod(const std::string& methodStr) {
    if (methodStr == "GET") return (methodStr);
    if (methodStr == "POST") return (methodStr);
    if (methodStr == "DELETE") return (methodStr);

    // Log unsupported method for debugging
    std::cerr << "Unsupported HTTP method: " << methodStr << std::endl;
    throw std::runtime_error("Unsupported HTTP method: " + methodStr);

	// do a 400 BAD REQUEST ERROR
}


// Updated parseHeaders function to store headers in a case-insensitive manner
std::map<std::string, std::string> HttpRequest::parseHeaders(std::istringstream& requestStream)
{
	std::map<std::string, std::string> headers;
	std::string line;

	while (std::getline(requestStream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back(); // Remove trailing \r
		if (line.empty())
			break; // End of headers
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
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
	return (headers);
}


std::string HttpRequest::getHeader(const std::string& key) const
{
    std::string normalizedKey = key;
    std::transform(normalizedKey.begin(), normalizedKey.end(), normalizedKey.begin(), ::tolower);
    auto it = headers.find(normalizedKey);
    if (it != headers.end()) {
        return it->second;
    }
    return ""; // Return empty string if header not found
}

std::string HttpRequest::getMethod() const
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



// std::map<std::string, std::string> parseBody(const std::string &body) {
//     std::map<std::string, std::string> keyValueMap;
//     size_t start = 0, end = 0;

//     // Split body by '&' to extract key-value pairs
//     while ((end = body.find('&', start)) != std::string::npos) {
//         std::string pair = body.substr(start, end - start);
//         size_t equalPos = pair.find('=');
//         if (equalPos != std::string::npos) {
//             std::string key = pair.substr(0, equalPos);
//             std::string value = pair.substr(equalPos + 1);
//             keyValueMap[key] = value; // Add key-value pair to map
//         }
//         start = end + 1;
//     }

//     // Handle the last pair (or the only pair if no '&' was found)
//     std::string pair = body.substr(start);
//     size_t equalPos = pair.find('=');
//     if (equalPos != std::string::npos) {
//         std::string key = pair.substr(0, equalPos);
//         std::string value = pair.substr(equalPos + 1);
//         keyValueMap[key] = value;
//     }

//     return keyValueMap;
// }