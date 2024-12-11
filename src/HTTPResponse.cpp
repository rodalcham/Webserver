#include "../include/Webserv.hpp"
#include "../include/HttpResponse.hpp"
#include "../include/ServerBlock.hpp"

class HttpRequest;

HttpResponse::HttpResponse(const HttpRequest& request)
{
	int	status_code_no;
	this->_http_version = request.getHttpVersion();
	this->_chunking_required = false;
	status_code_no = setFilePath(request);
	setStatusCode(status_code_no);
	parseBody();
	//	do error

}

HttpResponse::~HttpResponse()
{
	
}

int	HttpResponse::setFilePath(const HttpRequest& request)
{
	int	error_code = 200;
	this->_file_path = "root at the correct location" + request.getUri();
	//combine root at location(C) with uri(RQ) = file_path (saved in RS)
	//follow the file_path(RS) and check if method(RQ) is allowed at location(C) and if the file is accessible etc.
	// if there is an error on the way return error_code
	
	return (error_code);
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
		headers_list += "Content-Length: " + std::to_string(_body.length()) + "\r\n\r\n";

	return (headers_list);
}

void	HttpResponse::parseBody()
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
	else
	{
		this->_status_code = "404 Not Found";
		std::ifstream	file_404("./www/error_pages/404.html", std::ios::binary); // need to get this from the config file
		if (file_404.is_open())
		{
			buffer << file_404.rdbuf();
			std::string file_contents = buffer.str();
			file.close();
			this->_body = file_contents;
		}
		else
		{
			this->_body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>404 - Page Not Found</title></head><body><h1>404 - Page Not Found</h1><p>Oops! The page you\'re looking for doesn\'t exist or has been moved.</p></body></html>";
		}
	}
}

void	HttpResponse::sendResponse(int clientSock)
{
	std::string response;

	response =	this->_http_version + " " + _status_code + "\r\n" +
				this->getHeaderList() + "\r\n" +
				this->_body;

	write(clientSock, response.c_str(), response.length());
}

void	HttpResponse::setErrorResponse(const int& error_code)
{
	//function to set _file_path to the path of the error file based on what the config file says it should be
	setStatusCode(error_code);
	setErrorHeaders(error_code);
	parseBody();
}

void	HttpResponse::setStatusCode(const int& status_code_no)
{
	auto it = _error_status_codes.find(status_code_no);
	if (it != _error_status_codes.end())
		this->_status_code = it->second;
	else
		std::cout << "UNKNOWN ERROR CODE\n";// need to decide what to do in this situation!!!!!!!!!!!!!!!!!
}

void	HttpResponse::setErrorHeaders(const int& error_code)
{
	this->_headers["Content-Type"] = "text/html; charset=UTF-8";
	this->_headers["Connection"] = "close";
	if (error_code == 405 || error_code == 501)
		this->_headers["allowed"] = "GET PUT DELETE";// need to get this from the config file of the server
}

void	HttpResponse::setErrorBody(const int& error_code)
{
	
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



//         // Match the server block and set the block_index in HttpRequest
//         int i = matchServerBlock(httpRequest);
//         if (i < 0) {
//             std::cerr << "[ERROR] No matching server block found.\n";
//             close(clientSock);
//             return;
//         }
//         httpRequest.setBlockIndex(i); // Set the matched block index

//         const auto& locations = serverBlocks[i].location_blocks;
//         std::string matchedLocation = "/";
//         size_t longestMatch = 0;

//         // Updated matching logic
//         for (const auto& location : locations) {
//             if (httpRequest.getUri().find(location.first) == 0 && location.first.size() > longestMatch) {
//                 matchedLocation = location.first;
//                 longestMatch = location.first.size();
//             }
//         }

//         const auto& locationConfig = locations.at(matchedLocation);
//         std::string resolvedPath = resolvePath(httpRequest.getUri(), serverBlocks[i], locationConfig);
//         httpRequest.setFilePath(resolvedPath);


//         // Print allowed methods
//         if (locationConfig.find("allow_methods") != locationConfig.end()) {
//             std::cout << "[DEBUG] Allowed methods for location " << matchedLocation << ": ";
//             std::istringstream methods(locationConfig.at("allow_methods"));
//             std::string method;
//             while (methods >> method) {
//                 std::cout << method << " ";
//             }
//             std::cout << "\n";
//         }
//         std::cout << "=======================================\n";

//         // Check Allowed Methods
//         if (locationConfig.find("allow_methods") != locationConfig.end()) {
//             std::set<std::string> allowedMethods;
//             std::istringstream methods(locationConfig.at("allow_methods"));
//             std::string method;
//             while (methods >> method) {
//                 allowedMethods.insert(method);
//             }

//             if (allowedMethods.find(httpRequest.getMethod()) == allowedMethods.end()) {
//                 std::cerr << "[DEBUG] Method not allowed: " << httpRequest.getMethod() << " for location " << matchedLocation << "\n";
//                 HttpResponse response(httpRequest, 405, "Method Not Allowed");
//                 response.sendResponse(clientSock);
//                 close(clientSock);
//                 return;
//             }
//         }


// std::string Server::resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig) {
//     // Step 1: Determine the root directory
//     std::string rootDir;
//     // Check for location-specific root
//     if (locationConfig.find("root") != locationConfig.end()) {
//         rootDir = locationConfig.at("root");
//     }
//     // Check for server block root
//     else if (block.directive_pairs.find("root") != block.directive_pairs.end()) {
//         rootDir = block.directive_pairs.at("root");
//     }
//     // No root defined
//     else {
//         throw std::runtime_error("[ERROR] No root defined for the request."); // Return 404 later
//     }
//     // Step 2: Adjust the URI based on prefix
//     std::string strippedUri = uri;
//     if (locationConfig.find("prefix") != locationConfig.end()) {
//         std::string prefix = locationConfig.at("prefix");
//         if (uri.find(prefix) == 0) { // If the URI starts with the prefix
//             strippedUri = uri.substr(prefix.length()); // Remove the prefix from the URI
//         }
//     }
//     // Step 3: Construct the final path
//     std::string path = rootDir + strippedUri;
//     // Step 4: Security check to prevent directory traversal
//     if (path.find("..") != std::string::npos)
// 	{
//         throw std::runtime_error("[ERROR] Invalid path: Directory traversal attempt");
//     }
//     // Debugging information
//     std::cerr << "[DEBUG] Resolved path for URI " << uri << ": " << path << "\n";
//     return (path);
// }
