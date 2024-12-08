#include "../include/Webserv.hpp"
#include "../include/HTTPResponse.hpp"

class HttpRequest;

HttpResponse::HttpResponse(HttpRequest& request)
{
	this->httpVersion = request.getHttpVersion();

	//remove hardcoded values later - this is for testing only
	this->status_code = "200 OK";
	this->chunking_required = false;
	if (request.getUri() == "/")
		this->file_path = "./www/index.html";
	else
		this->file_path = "./www" + request.getUri();
}

HttpResponse::~HttpResponse()
{
	
}

std::string HttpResponse::getHeaderList()
{
	std::string		headers_list;

	for (const auto& pair : this->headers)
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

void	HttpResponse::sendResponse(int clientSock)
{
	std::string response;

	response =	this->httpVersion + " " + status_code + "\r\n" +
				this->getHeaderList() + "\r\n" +
				this->body;

	write(clientSock, response.c_str(), response.length());
}

void	HttpResponse::debug()
{
	std::cout << "\n =============== START LINE ===============\n\n";
	std::cout << "\n\n" << httpVersion << " " << status_code << "\n\n";

	std::cout << "\n =============== HEADERS ===============\n\n";
	std::cout << "\n\n" << getHeaderList() << "\n\n";

	std::cout << "\n =============== BODY ===============\n\n";
	std::cout << "\n\n" << body << "\n\n";
	
	std::cout << "\n\nfile_path: " << this->file_path << "\n";
}
