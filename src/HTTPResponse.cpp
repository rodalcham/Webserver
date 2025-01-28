#include "../include/Webserv.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/ServerBlock.hpp"

HttpResponse::HttpResponse()
{
	_ready = false;
}

HttpResponse::HttpResponse(const int& stat_code_no, const std::string &body, const HttpRequest &request) : _stat_code_no(stat_code_no), _body(body)
{
	// setReturnPage(request);
	this->_http_version = request.getHttpVersion();
	this->_chunking_required = false;
	if (this->_stat_code_no == 200 && request.getMethod() == "GET")
		setFilePath(request);
	setStatusCode(request);
	setBody();
	setHeaders(request);
	_ready = true;
	respDebug();
}

// void HttpResponse::setReturnPage(const HttpRequest& request)
// {
// 	_return_page = false;

// 	if ((_stat_code_no == 200 && request.getMethod() == "GET") || _stat_code_no == 401 || _stat_code_no == 403 || _stat_code_no == 404)// TODO: add others here (Hardcoded?)
// 	{
// 		_return_page = true;
// 	}
// }

bool	HttpResponse::isReady()
{
	return _ready;
}

HttpResponse::~HttpResponse()
{
	
}

HttpResponse&	HttpResponse::operator=(const HttpResponse& other)
{
	if (this != &other) // Check for self-assignment
	{
		// Copy primitive and standard types
		_http_version = other._http_version;
		_stat_code_no = other._stat_code_no;
		_status_code = other._status_code;
		_headers = other._headers;
		_chunking_required = other._chunking_required;
		_body = other._body;
		_file_path = other._file_path;
		_matched_location = other._matched_location;
		_return_page = other._return_page;
		_ready = other._ready;

		// _error_status_codes is a const map and is already initialized; no need to copy
	}
	return *this;
}

bool	HttpResponse::getReturnPage()
{
	return (this->_return_page);
}

void HttpResponse::setFilePath(const HttpRequest& request)
{
	const ServerBlock& block = request.getRequestBlock();

	_file_path = block.getDirectiveValue("root") + request.getUri();//change to root from request
}

void	HttpResponse::setStatusCode(HttpRequest request)
{
	auto it = _error_status_codes.find(this->_stat_code_no);
	if (it != _error_status_codes.end())
		this->_status_code = it->second;
	else
		std::cout << "UNKNOWN STATUS CODE\n";// TODO: need to decide what to do in this situation!!!!!!!!!!!!!!!!!
	if (_stat_code_no != 200 && _stat_code_no != 201 && _stat_code_no != 301)
		setErrorFilePath(request);
}

// void	HttpResponse::setBody(bool is_first_try, HttpRequest request)
// {
// 	setErrorFilePath(request);
// 	if (_file_path.empty())
// 		return;
// 	if (getReturnPage())//maybe can remove this??????????????????????????????????
// 	{
// 		std::stringstream	buffer;
// 		std::ifstream		file(this->_file_path, std::ios::binary);

// 		if (file.is_open())
// 		{
// 			buffer << file.rdbuf();
// 			std::string file_contents = buffer.str();
// 			file.close();
// 			this->_body = file_contents;
// 		}
// 		else if (is_first_try)
// 		{
// 			if (_stat_code_no == 200)
// 				_stat_code_no = 404;
// 			setStatusCode(request);
// 			setBody(false, request);
// 		}
// 		else
// 			this->_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>" + _status_code + "</title></head><body><h1>" + _status_code + "</h1></body></html>"; // TODO: need to complete this with a basic html page
// 	}
// 	else
// 		this->_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>" + _status_code + "</title></head><body><h1>" + _status_code + "</h1></body></html>"; // TODO: need to complete this with a basic html page
// }

// void	HttpResponse::setBody(bool is_first_try, HttpRequest request)
// {
	
// 	if (!_body.empty())
// 		return;
// 	if (getReturnPage())
// 	{
// 		std::stringstream	buffer;
// 		std::ifstream		file(this->_file_path, std::ios::binary);

// 		if (file.is_open())
// 		{
// 			buffer << file.rdbuf();
// 			std::string file_contents = buffer.str();
// 			file.close();
// 			this->_body = file_contents;
// 		}
// 		else if (is_first_try)
// 		{
// 			if (_stat_code_no == 200)
// 				_stat_code_no = 404;
// 			setStatusCode(request);
// 			setBody(false, request);
// 		}
// 		else
// 			this->_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>" + _status_code + "</title></head><body><h1>" + _status_code + "</h1></body></html>"; // TODO: need to complete this with a basic html page
// 	}
// 	else
// 		this->_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>" + _status_code + "</title></head><body><h1>" + _status_code + "</h1></body></html>"; // TODO: need to complete this with a basic html page
// }

void	HttpResponse::setBody()
{
	
	if (_stat_code_no == 200 || _stat_code_no == 201)
		return;
	else if (!_file_path.empty())
	{
		std::stringstream	buffer;
		std::ifstream		file(this->_file_path, std::ios::binary);

		if (file.is_open())
		{
			buffer << file.rdbuf();
			std::string file_contents = buffer.str();
			file.close();
			this->_body = file_contents;
			return;
		}
	}
	std::string		page_content = _body;
	_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>" + _status_code + "</title></head><body><h1>" + _body + "</h1></body></html>";

}

void	HttpResponse::setHeaders(const HttpRequest& request)
{
	this->_headers["Server"] = "webserv/42.0";
	this->_headers["Date"] = this->setDateHeader();
	this->_headers["Connection"] = "keep-alive";

	if (_stat_code_no == 200 && request.getMethod() == "GET")
	{
		this->_headers["Last-Modified"] = this->setLastModifiedHeader(request);
		this->_headers["Content-Type"] = this->setMimeTypeHeader();
	}
	else if (_stat_code_no == 201 && request.getMethod() == "POST")
	{
		this->_headers["Content-Type"] = request.getHeader("Content-Type");
		this->_headers["Location"] = request.getMatched_location();
	}
	else if (_stat_code_no == 301)
	{
		this->_headers["Location"] = request.getRedirLocation();
	}
	else
	{
		this->_headers["Content-Type"] = "text/html; charset=UTF-8";
		this->_headers["Connection"] = "close";
		if (_stat_code_no == 405)
			this->_headers["allowed"] = request.getAllowedMethods();
	}
}

void	HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	this->_headers[key] = value;
}

std::string HttpResponse::getHeaderList()
{
	std::string		headers_list;

	for (const auto& pair : this->_headers)
	{
		// std::cout << "Key: -->" << pair.first << "<-- Value: -->" << pair.second << "<--\n";
		headers_list += pair.first + ": " + pair.second + "\r\n";
		if (pair.first == "Transfer-Encoding")
			this->_chunking_required = true;
	}
	if (!_chunking_required)
		headers_list += "Content-Length: " + std::to_string(_body.length()) + "\r\n";// TODO: this needs to be looked at

	return (headers_list);
}

std::string HttpResponse::getFilePath()
{
	return (this->_file_path);
}

std::string	HttpResponse::returnResponse()
{
	std::string response;

	response =	this->_http_version + " " + _status_code + "\r\n" +
				this->getHeaderList() + "\r\n" +
				this->_body + "\r\n\r\n";

	return (response);
}

std::string	HttpResponse::makeTimestampStr(std::tm* time)
{
	std::ostringstream timestamp_stream;

	timestamp_stream << std::put_time(time, "%a") << ", "
					<< std::put_time(time, "%d %b %Y ")
					<< std::put_time(time, "%X %Z");

	return (timestamp_stream.str());
}

std::string	HttpResponse::setDateHeader()
{
	std::time_t time_since_epoch = std::time(nullptr);
	std::tm* current_date_obj = std::localtime(&time_since_epoch);

	std::string current_date_str = makeTimestampStr(current_date_obj);

	return (current_date_str);
}

std::string	HttpResponse::setLastModifiedHeader(HttpRequest request)
{
	std::filesystem::file_time_type lw_time = std::filesystem::last_write_time(this->_file_path);
	if (request.getHeader("X-uploadEndpoint") != "")
		lw_time = std::filesystem::last_write_time(request.getHeader("root") + request.getHeader("X-uploadEndpoint"));
	std::time_t sctp = decltype(lw_time)::clock::to_time_t(lw_time);
	std::tm* last_modified_obj = std::localtime(&sctp);

	std::string last_modified_str = makeTimestampStr(last_modified_obj);

	return (last_modified_str);
}

std::string HttpResponse::setMimeTypeHeader()
{
	size_t dotPos = this->_file_path.find_last_of('.');

	if (dotPos == std::string::npos)
		return "application/octet-stream";

	std::string extension = this->_file_path.substr(dotPos + 1);

	if (extension == "html" || extension == "htm")
		return "text/html";
	if (extension == "css")
		return "text/css";
	if (extension == "js")
		return "application/javascript";
	if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	if (extension == "png")
		return "image/png";
	if (extension == "gif")
		return "image/gif";
	if (extension == "txt")
		return "text/plain";

	return "application/octet-stream";
}

void	HttpResponse::respDebug()
{
	std::cout << "\n =============== START LINE ===============\n\n";
	std::cout << "\n\n" << _http_version << " " << _status_code << "\n\n";

	std::cout << "\n =============== HEADERS ===============\n\n";
	std::cout << "\n\n" << getHeaderList() << "\n\n";

	std::cout << "\n =============== BODY ===============\n\n";
	std::cout << "\n\n" << _body << "\n\n";

	std::cout << "\n\nfile_path: " << this->_file_path << "\n";

	std::cout << "\n -----------------------------------------------------------\n\n";

}

void HttpResponse::setErrorFilePath(const HttpRequest& request)
{
	const ServerBlock& block = request.getRequestBlock();
	std::string error_code_str = block.getErrorPageValue(std::to_string(_stat_code_no));

	if (!error_code_str.empty())
		_file_path = block.getLocationValue(request.getMatched_location(), "root") + error_code_str;
	else
		_file_path = "";
}
