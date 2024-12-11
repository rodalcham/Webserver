#include "../include/Webserv.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/ServerBlock.hpp"

class HttpRequest;

HttpResponse::HttpResponse(const HttpRequest& request)
{
	this->_http_version = request.getHttpVersion();
	this->_chunking_required = false;
	int status_code_no = setFilePath(request);
	setStatusCode(status_code_no);
	setBody(true);
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















void	HttpResponse::setErrorFilePath(const int& error_code_no)
{
	this->_file_path = "www/error_pages/" + std::to_string(error_code_no) + ".html";// TODO: make this re-set the file path based on the error
}




void	HttpResponse::setStatusCode(const int& status_code_no)
{
	auto it = _error_status_codes.find(status_code_no);
	if (it != _error_status_codes.end())
		this->_status_code = it->second;
	else
		std::cout << "UNKNOWN STATUS CODE\n";// TODO: need to decide what to do in this situation!!!!!!!!!!!!!!!!!
	if (status_code_no != 200 && status_code_no != 201)
		setErrorFilePath(status_code_no);// TODO: do this -----------------------------------
}

void	HttpResponse::setBody(bool is_first_try)
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
		setStatusCode(404);
		setBody(false);
	}
	else
		this->_body = "404 Not Found"; // TODO: need to complete this with a basic html page
}

void	HttpResponse::setHeaders(const int& status_code_no, const HttpRequest& request)
{
	if (status_code_no == 200 && request.getMethod() == "GET")
	{

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

std::string	HttpResponse::returnResponse()
{
	std::string response;

	response =	this->_http_version + " " + _status_code + "\r\n" +
				this->getHeaderList() + "\r\n" +
				this->_body;

	return (response);
}

void	HttpResponse::debug()
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
