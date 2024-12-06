#include "../include/Webserv.hpp"
#include "../include/HTTPResponse.hpp"

class HttpRequest;

HttpResponse::HttpResponse(HttpRequest& request)
{
	this->httpVersion = request.get_httpVersion();

	//remove hardcoded values later - this is for testing only
	this->status_code = "200 OK";
	// this->headers["server"] = "localhost";
	// this->headers["content-Type"] = request.get_header("accept");
	this->headers["content-Type"] = "text/html; charset=UTF-8";
	this->headers["connection"] = request.get_header("connection");
	this->chunking_required = false;
	if (request.get_uri() == "/")
		this->file_path = "./www/index.html";
	else
		this->file_path = "./www" + request.get_uri();
	this->parseBody();
}

HttpResponse::~HttpResponse()
{
	
}

void	HttpResponse::sendResponse(int clientSock)
{
	std::string response;

	response =	this->httpVersion + " " + status_code + "\r\n" +
				this->get_header_list() + "\r\n" +
				this->body;

	write(clientSock, response.c_str(), response.length());
}

void	HttpResponse::parseBody()
{
	std::stringstream	buffer;
	std::ifstream		file(this->file_path, std::ios::binary);

	if (file.is_open())
	{
		buffer << file.rdbuf();
		std::string file_contents = buffer.str();
		file.close();
		this->body = file_contents;
	}
	else
	{
		this->status_code = "404 Not Found";
		std::ifstream	file_404("./www/error_pages/404.html", std::ios::binary); // need to get this from the config file
		if (file.is_open())
		{
			buffer << file_404.rdbuf();
			std::string file_contents = buffer.str();
			file.close();
			this->body = file_contents;
		}
	}
}

std::string HttpResponse::get_header_list()
{
	std::string		headers_list;

	for (const auto& pair : headers)
	{
		// std::cout << "Key: -->" << pair.first << "<-- Value: -->" << pair.second << "<--\n";
		headers_list += pair.first + ": " + pair.second + "\r\n";
		if (pair.first == "Transfer-Encoding")
			this->chunking_required = true;
	}
	if (!chunking_required)
		headers_list += "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n";

	return (headers_list);
}


void	HttpResponse::debug()
{
	std::cout << "\n =============== START LINE ===============\n\n";
	std::cout << "\n\n" << httpVersion << " " << status_code << "\n\n";

	std::cout << "\n =============== HEADERS ===============\n\n";
	std::cout << "\n\n" << get_header_list() << "\n\n";

	std::cout << "\n =============== BODY ===============\n\n";
	std::cout << "\n\n" << body << "\n\n";
	
	std::cout << "\n\nfile_path: " << this->file_path << "\n";
}
