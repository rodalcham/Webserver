#include "../include/Webserv.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/ServerBlock.hpp"
#include "../include/cgi.hpp"

// Constructors

// Default Constructor
HttpRequest::HttpRequest() 
	: _stat_code_no(200), _filename(""), _request_block(NULL)
{
	// Default initialized request
}

// Parameterized Constructor with Pre-Matched ServerBlock
HttpRequest::HttpRequest(Client &client)
	: _stat_code_no(200), _filename(""), _request_block(client.getServerBlock())
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

	// If POST and status is 200, we can try processing the body if already available
	if (_method == "POST" && _stat_code_no == 200) {
		if (!_body.empty()) {
			try {
				processBody(_body);
				if (_stat_code_no == 200)
					_stat_code_no = 201;
			} catch (const std::runtime_error& e) {
				std::cerr << "[DEBUG] Error processing body: " << e.what() << "\n";
				return;
			}
		}
	}

	finalizeRequest();
}

HttpRequest::HttpRequest(const std::string& request, const ServerBlock *requestBlock)
    : _stat_code_no(200), _filename(""), _request_block(requestBlock)
{
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

	// If POST and status is 200, we can try processing the body if already available
	if (_method == "POST" && _stat_code_no == 200) {
		if (!_body.empty()) {
			try {
				processBody(_body);
				if (_stat_code_no == 200)
					_stat_code_no = 201;
			} catch (const std::runtime_error& e) {
				std::cerr << "[DEBUG] Error processing body: " << e.what() << "\n";
				return;
			}
		}
	}

	finalizeRequest();
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

// Unchunk Body for Chunked Transfer-Encoding
std::string HttpRequest::unchunkBody(const std::string& body)
{
	std::string result;
	size_t pos = 0;

	while (pos < body.size()) {
		size_t chunkSizeEnd = body.find("\r\n", pos);
		if (chunkSizeEnd == std::string::npos) break;

		std::string chunkSizeStr = body.substr(pos, chunkSizeEnd - pos);
		size_t chunkSize;
		try
		{
			chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
		}
		catch (...)
		{
			throw std::runtime_error("Invalid chunk size");
		}

		if (chunkSize == 0) break; // End of chunked body

		pos = chunkSizeEnd + 2; // Move past the chunk size line
		if (pos + chunkSize > body.size())
		{
			throw std::runtime_error("Chunk size exceeds body size");
		}

		result += body.substr(pos, chunkSize);
		pos += chunkSize + 2; // Move past the chunk data and trailing \r\n
	}

	return result;
}

// Process Transfer-Encoding or Content-Length
void HttpRequest::processBody(const std::string& body)
{
	auto transfer_encoding = getHeaders("transfer-encoding");
	if (!transfer_encoding.empty() && transfer_encoding == "chunked")
	{
		_body = unchunkBody(body);
	}
	else
	{
		auto content_length_str = getHeaders("content-length");
		if (!content_length_str.empty())
		{
			size_t content_length = std::stoul(content_length_str);
			if (body.size() != content_length)
			{
				_stat_code_no = 400; // Bad Request
				throw std::runtime_error("Body length does not match Content-Length header");
			}
		}
		_body = body;
	}
}

// Parse Multipart Filename
void HttpRequest::parseMultipartFilename()
{
	if (_headers.find("content-type") == _headers.end() ||
		_headers["content-type"].find("multipart/form-data") == std::string::npos)
	{
		throw std::runtime_error("Content-Type is not multipart/form-data");
	}

	size_t boundary_pos = _headers["content-type"].find("boundary=");
	if (boundary_pos == std::string::npos) {
		throw std::runtime_error("Boundary not found in Content-Type header");
	}

	std::string boundary = "--" + _headers["content-type"].substr(boundary_pos + 9);
	boundary.erase(boundary.find_last_not_of(" \t\r\n") + 1); // Trim whitespace

	size_t boundaryPos = _body.find(boundary);
	if (boundaryPos == std::string::npos)
	{
		throw std::runtime_error("Boundary not found in multipart body");
	}

	size_t partStart = _body.find("\r\n", boundaryPos) + 2;
	while (partStart != std::string::npos && partStart < _body.size())
	{
		size_t partEnd = _body.find(boundary, partStart);
		if (partEnd == std::string::npos)
		{
			throw std::runtime_error("Missing boundary in multipart body");
		}

		size_t headerEnd = _body.find("\r\n\r\n", partStart);
		if (headerEnd == std::string::npos || headerEnd + 4 > partEnd)
		{
			throw std::runtime_error("Invalid multipart headers");
		}

		std::string partHeaders = _body.substr(partStart, headerEnd - partStart);

		size_t filenamePos = partHeaders.find("filename=");
		if (filenamePos != std::string::npos) {
			size_t startQuote = partHeaders.find('"', filenamePos);
			size_t endQuote = partHeaders.find('"', startQuote + 1);
			if (startQuote != std::string::npos && endQuote != std::string::npos)
			{
				_filename = partHeaders.substr(startQuote + 1, endQuote - startQuote - 1);

				// Extract file content: from headerEnd + 4 up to partEnd - 2 (before \r\n)
				size_t contentStart = headerEnd + 4;
				size_t contentEnd = partEnd - 2; // exclude the \r\n before boundary
				_file_content = _body.substr(contentStart, contentEnd - contentStart);
				return;
			}
			else
			{
				throw std::runtime_error("Quotes not found around filename");
			}
		}

		partStart = _body.find("\r\n", partEnd + boundary.size()) + 2;
	}

	throw std::runtime_error("Filename not found in multipart body");
}

// Finalize Request (Simplified)
void HttpRequest::finalizeRequest()
{
	if (_stat_code_no != 200 && _stat_code_no != 201) return; // Only finalize if success

	if ((_method == "POST") && 
		getHeaders("content-type").find("multipart/form-data") != std::string::npos)
	{
		try
		{
			parseMultipartFilename();
			std::cerr << "[DEBUG] Successfully parsed multipart data.\n";
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << "[DEBUG] Error parsing multipart data: " << e.what() << "\n";
			_stat_code_no = 400;
			return;
		}
	}

	// Since _request_block is already set, no need to match
	// Additional finalization logic can be added here if necessary

	std::cerr << "[DEBUG] Finalized request with matched ServerBlock.\n";
}

// Parse Body with Pre-Matched ServerBlock
void HttpRequest::parseBody(Client& client)
{
	std::string& partialBody = client.getPartialRequestBody();
	auto content_length_str = getHeaders("content-length");
	auto transfer_encoding = getHeaders("transfer-encoding");

	if (content_length_str.empty() && transfer_encoding.empty())
	{
		return;
	}
	if (!content_length_str.empty())
	{
		size_t expectedLength = 0;
		try
		{
			expectedLength = std::stoul(content_length_str);
		}
		catch (...)
		{
			_stat_code_no = 400; // Bad Request
			return;
		}

		// Check if the full body has been received
		if (partialBody.size() < expectedLength)
		{
			return; // Wait for more data
		}

		_body = partialBody.substr(0, expectedLength);

		// Process the body content
		try
		{
			processBody(_body);
			std::cerr << "[DEBUG] Status after processBody(): " << _stat_code_no << "\n";
			// Do not set 201 here. We'll set it after writing the file.
		}
		catch (const std::exception& e)
		{
			std::cerr << "[DEBUG] Exception in processBody(): " << e.what() << "\n";
			std::cerr << "[DEBUG] Status after exception in processBody(): " << _stat_code_no << "\n";
			return;
		}
		catch (...)
		{
			std::cerr << "[DEBUG] Unknown exception in processBody()\n";
			std::cerr << "[DEBUG] Status after unknown exception: " << _stat_code_no << "\n";
			return;
		}

		// Finalize request (e.g., parse multipart filename if needed)
		finalizeRequest();

		// If still successful and method is POST, attempt to write the file
		if (_stat_code_no == 200 && _method == "POST")
		{
			std::string filename = getFilename();
			if (filename.empty())
			{
				filename = "uploaded_file"; // fallback if no filename from multipart
				std::cerr << "[DEBUG] Filename not found in multipart data. Using default filename: " << filename << "\n";
			}
			else
			{
				std::cerr << "[DEBUG] Filename extracted: " << filename << "\n";
			}

			std::string filePath = "www/uploads/" + filename;
			std::ofstream outFile(filePath, std::ios::binary);
			if (!outFile.is_open())
			{
				std::cerr << "[DEBUG] Failed to open file for writing: " << filePath << "\n";
				_stat_code_no = 500; // Internal Server Error
				std::cerr << "[DEBUG] Status after file open failure: " << _stat_code_no << "\n";
			}
			else
			{
				outFile.write(_file_content.data(), _file_content.size());
				if (outFile.fail()) {
					std::cerr << "[DEBUG] Failed to write data to file: " << filePath << "\n";
					_stat_code_no = 500; // Internal Server Error
				}
				else
				{
					outFile.close();
					std::cerr << "[DEBUG] File saved: " << filePath << "\n";
					// Now that we've successfully saved the file, set status to 201
					_stat_code_no = 201;
					std::cerr << "[DEBUG] Status after successful file write: " << _stat_code_no << "\n";
				}
			}

			// Send final response based on final status
			if (_stat_code_no == 201)
			{
				std::string resp = "HTTP/1.1 201 Created\r\nContent-Length:0\r\n\r\n";
				ssize_t sent = send(client.getSocket(), resp.c_str(), resp.size(), 0);
				if (sent < 0) {
					std::cerr << "[ERROR] Failed to send 201 Created response: " << strerror(errno) << "\n";
				} else {
					std::cerr << "[DEBUG] Sent 201 Created response. Clearing request.\n";
				}
				client.clearPartialRequest();
			}
			else
			{
				// If something went wrong
				if (_stat_code_no == 500)
				{
					std::string resp = "HTTP/1.1 500 Internal Server Error\r\nContent-Length:0\r\n\r\n";
					ssize_t sent = send(client.getSocket(), resp.c_str(), resp.size(), 0);
					if (sent < 0)
					{
						std::cerr << "[ERROR] Failed to send 500 Internal Server Error response: " << strerror(errno) << "\n";
					}
					else
					{
						std::cerr << "[DEBUG] Sent 500 Internal Server Error response. Clearing request.\n";
					}
					client.clearPartialRequest();
				}
			}
		}
		else
		{
			// If not 200 or method not POST, do not attempt file writing
			std::cerr << "[DEBUG] Not writing file. Status: " << _stat_code_no
					<< ", Method: " << _method << "\n";
		}
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
