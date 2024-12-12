#include "../include/Webserv.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/ServerBlock.hpp"
#include "../include/cgi.hpp"

HttpRequest::HttpRequest(const std::string& request, std::vector<ServerBlock>& serverBlocks)
{
    // Parse the request
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request: Missing headers or body");
    }

    std::string headerPart = request.substr(0, headerEnd);
    this->_body = request.substr(headerEnd + 4);

    std::istringstream requestStream(headerPart);
    std::string requestLine;
    std::getline(requestStream, requestLine);
    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    size_t methodEnd = requestLine.find(' ');
    if (methodEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request line: Missing method");
    }
    this->_method = parseHttpMethod(requestLine.substr(0, methodEnd));

    size_t uriEnd = requestLine.find(' ', methodEnd + 1);
    if (uriEnd == std::string::npos) {
        throw std::runtime_error("Invalid HTTP request line: Missing URI");
    }
    this->_uri = requestLine.substr(methodEnd + 1, uriEnd - methodEnd - 1);
    this->_http_version = requestLine.substr(uriEnd + 1);

    this->_headers = parseHeaders(requestStream);

    if (_headers.find("transfer-encoding") != _headers.end() &&
        _headers["transfer-encoding"] == "chunked") {
        this->_body = unchunkBody(_body);
    }
    this->_request_block = matchServerBlock(serverBlocks);
}

// Destructor
HttpRequest::~HttpRequest()
{
	// Clean up resources if needed
}

ServerBlock* HttpRequest::matchServerBlock(std::vector<ServerBlock>& serverBlocks) {
    std::string host = this->getHeader("host");
    std::cout << "[DEBUG] Matching server block for host: " << host << "\n";

    for (auto& block : serverBlocks) {
        auto it = block.directive_pairs.find("server_name");
        if (it != block.directive_pairs.end() && it->second == host) {
            std::cout << "[DEBUG] Matched server block for host: " << host << "\n";
            return (&block); // Return a non-const reference
        }
    }

    throw std::runtime_error("No matching server block found for host: " + host);
}




std::string HttpRequest::parseHttpMethod(const std::string& methodStr) {
    if (methodStr == "GET") return (methodStr);
    if (methodStr == "POST") return (methodStr);
    if (methodStr == "DELETE") return (methodStr);

    std::cerr << "Unsupported HTTP method: " << methodStr << std::endl;
    throw std::runtime_error("Unsupported HTTP method: " + methodStr);

	// do a 400 BAD REQUEST ERROR
}

std::map<std::string, std::string> HttpRequest::parseHeaders(std::istringstream& requestStream)
{
	std::map<std::string, std::string> headers;
	std::string line;

	while (std::getline(requestStream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			break;
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string headerName = line.substr(0, colonPos);
			std::string headerValue = line.substr(colonPos + 1);

			headerName.erase(0, headerName.find_first_not_of(" \t\r\n"));
			headerName.erase(headerName.find_last_not_of(" \t\r\n") + 1);

			headerValue.erase(0, headerValue.find_first_not_of(" \t\r\n"));
			headerValue.erase(headerValue.find_last_not_of(" \t\r\n") + 1);

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
    auto it = _headers.find(normalizedKey);
    if (it != _headers.end()) {
        return it->second;
    }
    return ""; // Return empty string if header not found
}

std::string HttpRequest::getMethod() const
{
	return _method;
}

std::string HttpRequest::getUri() const
{
	return _uri;
}

std::string HttpRequest::getHttpVersion() const
{
	return _http_version;
}

std::string HttpRequest::getBody() const
{
	return _body;
}

std::string HttpRequest::getHeaders(const std::string& key) const
{
	auto it = _headers.find(key);
	if (it != _headers.end()) {
		return it->second;
	}
	return "";
}

// Print the details of the HTTP request
void HttpRequest::rqstDebug() const {
    std::cout << "Received HTTP Request:\n";
    std::cout << "Header Connection: " << getHeaders("connection") << "\n\n";
    std::cout << "Header accept: " << getHeaders("accept") << "\n\n";
    // std::cout << "Method: " << get_method() << "\n";
    std::cout << "URI: " << getUri() << "\n";
    std::cout << "HTTP Version: " << getHttpVersion() << "\n";
    std::cout << "Body: " << getBody() << "\n\n";
}

std::string HttpRequest::unchunkBody(const std::string& body) {
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
const ServerBlock& HttpRequest::getRequestBlock() const
{
    return *(_request_block);
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