#include "../include/Webserv.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/ServerBlock.hpp"

class HttpRequest;

HttpResponse::HttpResponse(const HttpRequest& request)
{
	this->_http_version = request.getHttpVersion();
	this->_chunking_required = false;
	int status_code_no = setFilePath(request);
	setStatusCode(status_code_no, request);
	setBody(true, request);
	setHeaders(status_code_no, request);
}

HttpResponse::~HttpResponse()
{
	
}

int HttpResponse::setFilePath(const HttpRequest& request)
{
	int status_code_no = 200;// TODO: make sure it is checking all subfolders and not just e.g. if you have /test/folder/uploads/ rather than /uploads/

	const ServerBlock& block = request.getRequestBlock();
	const std::map<std::string, std::map<std::string, std::string>>& locations = block.location_blocks;
	std::string matchedLocation = "/";
	size_t longestMatch = 0;
	for (const auto& location : locations)
	{
		if (request.getUri().find(location.first) == 0 && location.first.size() > longestMatch) {
			matchedLocation = location.first;
			longestMatch = location.first.size();
		}
	}
	if (locations.find(matchedLocation) == locations.end())
		return 404;
	const auto& locationConfig = locations.at(matchedLocation);
	try {
		this->_file_path = resolvePath(request.getUri(), block, locationConfig);
		if (locationConfig.find("allow_methods") != locationConfig.end())
		{
			std::set<std::string> allowedMethods;
			std::istringstream methods(locationConfig.at("allow_methods"));
			std::string method;
			while (methods >> method)
				allowedMethods.insert(method);
			if (allowedMethods.find(request.getMethod()) == allowedMethods.end())
				return 405;
		}
		std::ifstream file(this->_file_path);
		if (!file.good())
			return 404;
		file.close();
	}
	catch (const std::runtime_error& e)// TODO: is this the correct way to use try catch ?????????????????????
	{
		std::cerr << e.what() << "\n";
		return 400;
	}
	return status_code_no;
}





std::string HttpResponse::resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig)
{
    std::string rootDir;
    if (locationConfig.find("root") != locationConfig.end())
        rootDir = locationConfig.at("root");
    else if (block.directive_pairs.find("root") != block.directive_pairs.end())
        rootDir = block.directive_pairs.at("root");
    else
        throw std::runtime_error("[ERROR] No root defined for the request.");
    std::string strippedUri = uri;
    if (locationConfig.find("prefix") != locationConfig.end())
    {
        std::string prefix = locationConfig.at("prefix");
        if (uri.find(prefix) == 0)
        {
            strippedUri = uri.substr(prefix.length());
        }
    }
    std::string path = rootDir + strippedUri;
    if (path.find("..") != std::string::npos)
    {
        throw std::runtime_error("[ERROR] Invalid path: Directory traversal attempt");
    }
    std::cerr << "[DEBUG] Resolved path for URI " << uri << " | " << path << "\n";
    return path;
}








std::string HttpResponse::getFromLocation(const std::string& location, const std::string& key, const HttpRequest& request)
{
	// Get the server block from the HttpRequest
	const ServerBlock& block = request.getRequestBlock();

	// If a specific location is provided
	if (!location.empty())
	{
		const auto& locations = block.location_blocks;
		if (locations.find(location) == locations.end())
			throw std::runtime_error("Location '" + location + "' not found in the server block.");
		const auto& locationConfig = locations.at(location);

		// Extract the requested data from the location
		if (locationConfig.find(key) != locationConfig.end())
			return locationConfig.at(key);
		else
			throw std::runtime_error("Data '" + key + "' not found in the location '" + location + "'.");
	}

	// If no specific location is provided, look in the global directives
	if (block.directive_pairs.find(key) != block.directive_pairs.end())
		return block.directive_pairs.at(key);

	throw std::runtime_error("Data '" + key + "' not found in the global or location-specific configuration.");
}







void HttpResponse::setErrorFilePath(const int& error_code_no, HttpRequest request)
{
	std::string error_code_str = std::to_string(error_code_no);

	try {
		// Use get_from_location to fetch the error page configuration
		std::string errorPagePath = getFromLocation(this->_file_path, "error_page", request);

		// Construct the full path to the error page
		this->_file_path = resolvePath(errorPagePath + "/" + error_code_str + ".html", request.getRequestBlock(), {});
	} catch (const std::runtime_error& e) {
		// Log the error for debugging purposes
		std::cerr << "[ERROR] Error while setting error file path: " << e.what() << "\n";

	}
}





void	HttpResponse::setStatusCode(const int& status_code_no, HttpRequest request)
{
	auto it = _error_status_codes.find(status_code_no);
	if (it != _error_status_codes.end())
		this->_status_code = it->second;
	else
		std::cout << "UNKNOWN STATUS CODE\n";// TODO: need to decide what to do in this situation!!!!!!!!!!!!!!!!!
	if (status_code_no != 200 && status_code_no != 201)
		setErrorFilePath(status_code_no, request);// TODO: do this -----------------------------------
}

void	HttpResponse::setBody(bool is_first_try, HttpRequest request)
{
	std::stringstream	buffer;
	std::ifstream		file(this->_file_path, std::ios::binary);

	if (file.is_open())
	{
		buffer << file.rdbuf();
		std::string file_contents = buffer.str();
		file.close();
		this->_body = file_contents;
	}
	else if (is_first_try)
	{
		setStatusCode(404, request);
		setBody(false, request);
	}
	else
		this->_body = "404 Not Found"; // TODO: need to complete this with a basic html page
}

void	HttpResponse::setHeaders(const int& status_code_no, const HttpRequest& request)
{
	this->_headers["Server"] = "webserv/42.0";
	this->_headers["Date"] = this->setDateHeader();
	this->_headers["Connection"] = "keep-alive";

	if (status_code_no == 200 && request.getMethod() == "GET")
	{
		this->_headers["Last-Modified"] = this->setLastModifiedHeader();
		this->_headers["Content-Type"] = this->setMimeTypeHeader();
	}
	else if (status_code_no == 201 && request.getMethod() == "POST")
	{

	}
	else if (status_code_no == 200 && request.getMethod() == "DELETE")
	{

	}
	else
	{
		this->_headers["Content-Type"] = "text/html; charset=UTF-8";
		this->_headers["Connection"] = "close";
		if (status_code_no == 405 || status_code_no == 501)
			this->_headers["allowed"] = "GET PUT DELETE";// TODO: need to get this from the config file of the server
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
				this->_body;

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
	std::filesystem::file_time_type lw_time = std::filesystem::last_write_time(this->_file_path);

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

void	HttpResponse::rspDebug()
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
