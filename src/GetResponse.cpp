#pragma once

#include "../include/Webserv.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/GetResponse.hpp"

class HttpRequest;

GetResponse::GetResponse(HttpRequest& request) : HttpResponse(request)
{
	makeHeaderList(request);
	parseBody();
}

GetResponse::~GetResponse()
{
	
}

void GetResponse::makeHeaderList(HttpRequest& request)
{
	// this->headers["server"] = "localhost";
	// this->headers["content-Type"] = request.get_header("accept");
	this->_headers["content-Type"] = "text/html; charset=UTF-8";
	this->_headers["connection"] = request.getHeaders("connection");
}

void	GetResponse::parseBody()
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
		if (file.is_open())
		{
			buffer << file_404.rdbuf();
			std::string file_contents = buffer.str();
			file.close();
			this->_body = file_contents;
		}
	}
}
