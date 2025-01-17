#include "../include/Webserv.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/ServerBlock.hpp"

// Constructors

// Default Constructor
HttpRequest::HttpRequest() 
	: _stat_code_no(200), _filename(""), _request_block(NULL)
{
	// Default initialized request
}

// Parameterized Constructor with Pre-Matched ServerBlock
HttpRequest::HttpRequest(Client &client)
	: _stat_code_no(200), _filename(""), _request_block(client.getServerBlock()), _continue_response("")
{
	string	request = client.getRequest();

	size_t headerEnd = request.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		_stat_code_no = 400; // Bad Request
		throw std::runtime_error("Invalid HTTP request: Missing headers or body");
	}

	std::string headerPart = request.substr(0, headerEnd);
	_body = request.substr(headerEnd + 4); // The remainder is considered the body

	std::istringstream requestStream(headerPart);
	std::string requestLine;
	std::getline(requestStream, requestLine);
	if (!requestLine.empty() && requestLine.back() == '\r') {
		requestLine.pop_back();
	}

	size_t methodEnd = requestLine.find(' ');
	if (methodEnd == std::string::npos) {
		_stat_code_no = 400;
		throw std::runtime_error("Invalid HTTP request line: Missing method");
	}
	_method = parseHttpMethod(requestLine.substr(0, methodEnd));

	size_t uriEnd = requestLine.find(' ', methodEnd + 1);
	if (uriEnd == std::string::npos) {
		_stat_code_no = 400;
		std::cerr << "[DEBUG] Missing URI in the request line\n";
		throw std::runtime_error("Invalid HTTP request line: Missing URI");
	}
	_uri = requestLine.substr(methodEnd + 1, uriEnd - methodEnd - 1);
	_http_version = requestLine.substr(uriEnd + 1);

	// Parse headers
	_headers = parseHeaders(requestStream);
	headersGood(); 
	if (_stat_code_no == 100)
	{
		return;
	}

	// If _stat_code_no != 200 and method != POST, just return (likely error or non-body method)
	if (_stat_code_no != 200 && _method != "POST")
	{
		return;
	}
}


// Destructor
HttpRequest::~HttpRequest()
{
	// Cleanup if needed
}

// Parse HTTP Method
std::string HttpRequest::parseHttpMethod(const std::string& methodStr)
{
	if (methodStr == "GET") return methodStr;
	if (methodStr == "POST") return methodStr;
	if (methodStr == "DELETE") return methodStr;

	std::cerr << "Unsupported HTTP method: " << methodStr << std::endl;
	throw std::runtime_error("Unsupported HTTP method: " + methodStr);
}

// Parse Headers
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
	return headers;
}

// Validate Headers and Method
void HttpRequest::headersGood()
{
	if (_method == "GET")
	{
		_stat_code_no = 200;
	}
	else if (_method == "POST")
	{
		auto expect_header = getHeaders("expect");
		if (!expect_header.empty() && expect_header == "100-continue")
		{
			_stat_code_no = 100;
			_continue_response = "HTTP/1.1 100 Continue\r\n\r\n";
			return;
		}


		auto content_length_str = getHeaders("content-length");
		auto transfer_encoding = getHeaders("transfer-encoding");

		if (!content_length_str.empty() && !transfer_encoding.empty())
		{
			_stat_code_no = 400;
			return;
		}

		if (!content_length_str.empty())
		{
			try
			{
				size_t length = std::stoul(content_length_str);
				const size_t MAX_UPLOAD_SIZE = 50 * 1024 * 1024; // 50 MB max
				if (length > MAX_UPLOAD_SIZE) {
					_stat_code_no = 413; // Payload Too Large
					return;
				}
			}
			catch (...)
			{
				_stat_code_no = 400; // Bad Request
				return;
			}
		}

		_stat_code_no = 200;
	}
	else if (_method == "DELETE")
	{
		_stat_code_no = 200;
	}
}

// Get Header by Key (Case-Insensitive)
std::string HttpRequest::getHeader(const std::string& key) const
{
	std::string normalizedKey = key;
	std::transform(normalizedKey.begin(), normalizedKey.end(), normalizedKey.begin(), ::tolower);
	auto it = _headers.find(normalizedKey);
	if (it != _headers.end()) {
		return it->second;
	}
	return "";
}

// Getters
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

int HttpRequest::getStatusCode() const
{
	return _stat_code_no;
}

std::string HttpRequest::getFilename() const
{
	return _filename;
}

std::string HttpRequest::getFileContent() const
{
	return _file_content;
}

const ServerBlock& HttpRequest::getRequestBlock() const
{
	if (!_request_block) {
		throw std::runtime_error("Request block not set");
	}
	return *(_request_block);
}

const std::string& HttpRequest::getContinueResponse() const
{
	return _continue_response;
}

int HttpRequest::getPort() const
{
	return _port;
}

// Setters
void HttpRequest::setStatusCode(int statusCode)
{
	_stat_code_no = statusCode;
}

void HttpRequest::setFilename(const std::string& filename)
{
	_filename = filename;
}

void HttpRequest::setFileContent(const std::string& content)
{
	_file_content = content;
}

void HttpRequest::setPort(int port)
{
	_port = port;
}
