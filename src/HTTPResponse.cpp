// #include "../include/Webserv.hpp"
// #include "../include/HTTPResponse.hpp"
// #include "../include/HTTPRequest.hpp"
// #include <fstream>
// #include <sstream>
// #include <iostream>
// #include <unistd.h>
// #include <map>


// HttpResponse::HttpResponse() : status_code("200"), body("")
// {
//     headers["Content-Type"] = "text/plain";
//     headers["Content-Length"] = "0";
// }

// HttpResponse::HttpResponse(HttpRequest& request)
// {
// 	this->httpVersion = request.getHttpVersion();
// 	this->status_code = "200 OK";
// 	this->chunking_required = false;

// 	// Use the file path already set in HttpRequest
// 	// std::string requestedFile = request.getFilePath();
// 	// std::ifstream file(requestedFile, std::ios::binary);

// 	if (!file.is_open()) {
// 		// If for some reason file can't open here, fallback to 404:
// 		this->status_code = "404 Not Found";
// 		this->body = "File not found";
// 	} else {
// 		std::ostringstream content;
// 		content << file.rdbuf();
// 		this->body = content.str();
// 		this->headers["Content-Type"] = "text/html"; // Set a default, or detect MIME type if desired
// 	}
// }

// HttpResponse::HttpResponse(HttpRequest& request, int statusCode, const std::string& statusMessage)
// {
// 	this->httpVersion = request.getHttpVersion();
// 	this->status_code = std::to_string(statusCode) + " " + statusMessage;
// 	this->chunking_required = false;
// 	// For error or simple responses, just set a basic body.
// 	this->body = "<html><body><h1>" + std::to_string(statusCode) + " " + statusMessage + "</h1></body></html>";
// 	this->headers["Content-Type"] = "text/html";
// }

// HttpResponse::~HttpResponse() {}

// std::string HttpResponse::getHeaderList()
// {
// 	std::string headers_list;
// 	for (const auto& pair : this->headers) {
// 		headers_list += pair.first + ": " + pair.second + "\r\n";
// 		if (pair.first == "Transfer-Encoding")
// 			this->chunking_required = true;
// 	}
// 	if (!chunking_required)
// 		headers_list += "Content-Length: " + std::to_string(body.length()) + "\r\n";
// 	return headers_list;
// }

// void HttpResponse::sendResponse(int clientSock)
// {
// 	std::string response;
// 	response =	this->httpVersion + " " + status_code + "\r\n" +
// 				this->getHeaderList() + "\r\n" +
// 				this->body;

// 	write(clientSock, response.c_str(), response.size());
// }

// void HttpResponse::debug()
// {
// 	std::cout << "\n =============== START LINE ===============\n\n";
// 	std::cout << httpVersion << " " << status_code << "\n";

// 	std::cout << "\n =============== HEADERS ===============\n\n";
// 	std::cout << getHeaderList() << "\n";

// 	std::cout << "\n =============== BODY ===============\n\n";
// 	std::cout << body << "\n";
// }
// void HttpResponse::setBody(const std::string& bodyContent) {
//     body = bodyContent;
//     headers["Content-Length"] = std::to_string(body.size());
// }
// void HttpResponse::setHeader(const std::string& key, const std::string& value) {
//     headers[key] = value;
// }
