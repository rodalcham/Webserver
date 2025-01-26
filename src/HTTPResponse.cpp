#include "../include/Webserv.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/ServerBlock.hpp"

HttpResponse::HttpResponse(const HttpRequest& request) : _stat_code_no(request.getStatusCode())
{
	// if (this->_stat_code_no == 100)
	// {
	// 	_http_version = "HTTP/1.1";// TODO: REMOVE THIS once request is working
	// 	_status_code = "100 Continue";// TODO: REMOVE THIS once request is working
	// 	return;
	// }
	_ready = false;
	this->_http_version = request.getHttpVersion();
	this->_chunking_required = false;
	if (this->_stat_code_no == 200 || this->_stat_code_no == 201)
		setFilePath(request);
	setStatusCode(request);
	setBody(true, request);
	setHeaders(request);
}

HttpResponse::HttpResponse(const int& stat_code_no, const std::string& body, const HttpRequest &request) : _stat_code_no(stat_code_no), _body(body)
{
	setReturnPage(request);
	this->_http_version = request.getHttpVersion();
	this->_chunking_required = false;
	if (this->_stat_code_no == 200 && request.getMethod() == "GET")
		setFilePath(request);
	setStatusCode(request);
	setBody(true, request);
	setHeaders(request);
	_ready = true;
	// respDebug();
}

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


void HttpResponse::setFilePath(const HttpRequest& request) {
	const ServerBlock& block = request.getRequestBlock();

	_file_path = block.getDirectiveValue("root") + request.getUri();
}



void HttpResponse::setErrorFilePath(const HttpRequest& request) {
	const ServerBlock& block = request.getRequestBlock();
	std::string error_code_str = std::to_string(_stat_code_no);

	_file_path = block.getDirectiveValue("root") + block.getErrorPageValue(error_code_str);
	// try {
	// 	std::string errorPagePath;

	// 	// Check for location-specific error_page_<error_code>
	// 	try {
	// 		errorPagePath = getFromLocation(this->_matched_location, error_code_str, request);
	// 		// std::cerr << "[DEBUG] Using location-specific error_page: " << errorPagePath << "\n";
	// 	} catch (const std::runtime_error&) {
	// 		// Fallback to global _error_pages
	// 		if (block.getErrorPages().find(error_code_str) != block.getErrorPages().end()) {
	// 			errorPagePath = block.getErrorPages().at(error_code_str);
	// 			// std::cerr << "[DEBUG] Using global error_page: " << errorPagePath << "\n";
	// 		} else {
	// 			throw std::runtime_error("No error page found for code: " + error_code_str);
	// 		}
	// 	}

	// 	// Resolve path
	// 	this->_file_path = resolvePath(errorPagePath, block, {});
	// 	// std::cerr << "[DEBUG] Resolved error file path: " << this->_file_path << "\n";
	// } catch (const std::runtime_error& e) {
	// 	std::cerr << "[ERROR] Error while setting error file path: " << e.what() << "\n";
	// }
}


// void HttpResponse::setErrorFilePath(const HttpRequest& request) {
// 	std::string error_code_str = std::to_string(_stat_code_no);
// 	const ServerBlock& block = request.getRequestBlock();

// 	try {
// 		std::string errorPagePath;

// 		// Check for location-specific error_page_<error_code>
// 		try {
// 			errorPagePath = getFromLocation(this->_matched_location, error_code_str, request);
// 			// std::cerr << "[DEBUG] Using location-specific error_page: " << errorPagePath << "\n";
// 		} catch (const std::runtime_error&) {
// 			// Fallback to global _error_pages
// 			if (block.getErrorPages().find(error_code_str) != block.getErrorPages().end()) {
// 				errorPagePath = block.getErrorPages().at(error_code_str);
// 				// std::cerr << "[DEBUG] Using global error_page: " << errorPagePath << "\n";
// 			} else {
// 				throw std::runtime_error("No error page found for code: " + error_code_str);
// 			}
// 		}

// 		// Resolve path
// 		this->_file_path = resolvePath(errorPagePath, block, {});
// 		// std::cerr << "[DEBUG] Resolved error file path: " << this->_file_path << "\n";
// 	} catch (const std::runtime_error& e) {
// 		std::cerr << "[ERROR] Error while setting error file path: " << e.what() << "\n";
// 	}
// }







// int HttpResponse::setFilePath(const HttpRequest& request)
// {
// 	int status_code_no = 200;// TODO: make sure it is checking all subfolders and not just e.g. if you have /test/folder/uploads/ rather than /uploads/

// 	const ServerBlock& block = request.getRequestBlock();
// 	const std::map<std::string, std::map<std::string, std::string>>& locations = block.getAllLocationBlocks();
// 	std::string matchedLocation = "/";
// 	size_t longestMatch = 0;
// 	for (const auto& location : locations)
// 	{
// 		if (request.getUri().find(location.first) == 0 && location.first.size() > longestMatch) {
// 			matchedLocation = location.first;
// 			longestMatch = location.first.size();
// 		}
// 	}
// 	if (locations.find(matchedLocation) == locations.end())
// 		return 404;
// 	this->_matched_location = matchedLocation;
// 	const auto& locationConfig = locations.at(matchedLocation);
// 	try {
// 		this->_file_path = resolvePath(request.getUri(), block, locationConfig);
// 		if (locationConfig.find("allow_methods") != locationConfig.end())
// 		{
// 			std::set<std::string> allowedMethods;
// 			std::istringstream methods(locationConfig.at("allow_methods"));
// 			std::string method;
// 			while (methods >> method)
// 				allowedMethods.insert(method);
// 			if (allowedMethods.find(request.getMethod()) == allowedMethods.end())
// 				return 405;
// 		}
// 		std::ifstream file(this->_file_path);
// 		if (!file.good())
// 			return 404;
// 		file.close();
// 	}
// 	catch (const std::runtime_error& e)// TODO: is this the correct way to use try catch ?????????????????????
// 	{
// 		std::cerr << e.what() << "\n";
// 		return 400;
// 	}
// 	return status_code_no;
// }



// void HttpResponse::setErrorFilePath(const int& error_code_no, HttpRequest request) {
// 	std::string error_code_str = std::to_string(error_code_no);
// 	const ServerBlock& block = request.getRequestBlock();

// 	try {
// 		std::string errorPagePath;

// 		// Check for location-specific error_page_<error_code>
// 		try {
// 			errorPagePath = getFromLocation(this->_matched_location, error_code_str, request);
// 			// std::cerr << "[DEBUG] Using location-specific error_page: " << errorPagePath << "\n";
// 		} catch (const std::runtime_error&) {
// 			// Fallback to global _error_pages
// 			if (block.getErrorPages().find(error_code_str) != block.getErrorPages().end()) {
// 				errorPagePath = block.getErrorPages().at(error_code_str);
// 				// std::cerr << "[DEBUG] Using global error_page: " << errorPagePath << "\n";
// 			} else {
// 				throw std::runtime_error("No error page found for code: " + error_code_str);
// 			}
// 		}

// 		// Resolve path
// 		this->_file_path = resolvePath(errorPagePath, block, {});
// 		// std::cerr << "[DEBUG] Resolved error file path: " << this->_file_path << "\n";
// 	} catch (const std::runtime_error& e) {
// 		std::cerr << "[ERROR] Error while setting error file path: " << e.what() << "\n";
// 	}
// }





// std::string HttpResponse::resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig)
// {
//     std::string rootDir;
//     if (locationConfig.find("root") != locationConfig.end())
//         rootDir = locationConfig.at("root");
//     else if (block.getDirectivePairs().find("root") != block.getDirectivePairs().end())
//         rootDir = block.getDirectivePairs().at("root");
//     else
//         throw std::runtime_error("[ERROR] No root defined for the request.");
//     std::string strippedUri = uri;
//     if (locationConfig.find("prefix") != locationConfig.end())
//     {
//         std::string prefix = locationConfig.at("prefix");
//         if (uri.find(prefix) == 0)
//         {
//             strippedUri = uri.substr(prefix.length());
//         }
//     }
//     std::string path = rootDir + strippedUri;
// 	// std::cerr << "[DEBUG] Resolving path for URI '" << uri << "'\n";
// 	// std::cerr << "[DEBUG] Resolved path: " << path << "\n";

//     if (path.find("..") != std::string::npos)
//     {
//         throw std::runtime_error("[ERROR] Invalid path: Directory traversal attempt");
//     }
//     // std::cerr << "[DEBUG] Resolved path for URI " << uri << " | " << path << "\n";
//     return path;
// }

// std::string HttpResponse::getFromLocation(const std::string& location, const std::string& key, const HttpRequest& request) {
// 	const ServerBlock& block = request.getRequestBlock();

// 	// Check for location-specific configuration
// 	if (!location.empty() && block.getAllLocationBlocks().find(location) != block.getAllLocationBlocks().end()) {
// 		const auto& locationConfig = block.getAllLocationBlocks().at(location);

// 		// Check for location-specific error_page_<error_code>
// 		if (locationConfig.find("error_page_" + key) != locationConfig.end()) {
// 			// std::cerr << "[DEBUG] Found location-specific error_page for '" << key
// 					//   << "' in location '" << location << "' with value '"
// 					//   << locationConfig.at("error_page_" + key) << "'\n";
// 			return locationConfig.at("error_page_" + key);
// 		}

// 		// Check for other location-specific directives
// 		if (locationConfig.find(key) != locationConfig.end()) {
// 			// std::cerr << "[DEBUG] Found key '" << key << "' in location '" << location
// 					//   << "' with value '" << locationConfig.at(key) << "'\n";
// 			return locationConfig.at(key);
// 		}
// 	}

// 	// Check for global directives
// 	if (block.getDirectivePairs().find(key) != block.getDirectivePairs().end()) {
// 		return block.getDirectivePairs().at(key);
// 	}

// 	// Check for global _error_pages
// 	if (block.getErrorPages().find(key) != block.getErrorPages().end()) {
// 		return block.getErrorPages().at(key);
// 	}

// 	throw std::runtime_error("Data '" + key + "' not found in the location '" + location + "' or globally.");
// }






void	HttpResponse::setStatusCode(HttpRequest request)
{
	auto it = _error_status_codes.find(this->_stat_code_no);
	if (it != _error_status_codes.end())
		this->_status_code = it->second;
	else
		std::cout << "UNKNOWN STATUS CODE\n";// TODO: need to decide what to do in this situation!!!!!!!!!!!!!!!!!
	if (_stat_code_no != 200 && _stat_code_no != 201)
		setErrorFilePath(request);// TODO: do this -----------------------------------
}

void	HttpResponse::setBody(bool is_first_try, HttpRequest request)
{
	std::stringstream	buffer;
	std::ifstream		file(this->_file_path, std::ios::binary);


	// std::cout << _file_path << "<<<<<<<<<<<<<<<<<<<<<<<<-----file path\n";

	if (file.is_open())
	{
		buffer << file.rdbuf();
		std::string file_contents = buffer.str();
		file.close();
		this->_body = file_contents;
	}
	else if (is_first_try)
	{
		_stat_code_no = 404;
		setStatusCode(request);
		setBody(false, request);
	}
	else
		this->_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>"; // TODO: need to complete this with a basic html page
}


void	HttpResponse::setHeaders(const HttpRequest& request)
{
	this->_headers["Server"] = "webserv/42.0";
	this->_headers["Date"] = this->setDateHeader();
	this->_headers["Connection"] = "keep-alive";

	if (_stat_code_no == 200 && request.getMethod() == "GET")
	{
		this->_headers["Last-Modified"] = this->setLastModifiedHeader();
		this->_headers["Content-Type"] = this->setMimeTypeHeader();
	}
	else if (_stat_code_no == 201 && request.getMethod() == "POST")
	{

	}
	else if (_stat_code_no == 200 && request.getMethod() == "DELETE")
	{

	}
	else
	{
		this->_headers["Content-Type"] = "text/html; charset=UTF-8";
		this->_headers["Connection"] = "close";
		if (_stat_code_no == 405 || _stat_code_no == 501)
			this->_headers["allowed"] = "TODO: this is temp GET POST DELETE";// TODO: need to get this from the config file of the server
	}
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
		headers_list += "Content-Length: " + std::to_string(_body.length()) + "\r\n\r\n";// TODO: this needs to be looked at

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

std::string	HttpResponse::setLastModifiedHeader()
{
	// std::filesystem::file_time_type lw_time = std::filesystem::last_write_time(this->_file_path);

	// std::time_t sctp = decltype(lw_time)::clock::to_time_t(lw_time);
	// std::tm* last_modified_obj = std::localtime(&sctp);
	
	// std::string last_modified_str = makeTimestampStr(last_modified_obj);
	std::string last_modified_str = "";

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
