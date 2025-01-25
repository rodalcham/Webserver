#include "../include/Webserv.hpp"
#include "../include/Server.hpp"
#include "../include/Config.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/Client.hpp"

#include <errno.h>
#include <filesystem> // C++17 or later
#include <vector>
#include <string>

class log;

bool	isHttpRequest(const std::string &request);
string	listUploadsJSON(const std::string &, string endpoint);

bool	checkRequestSize(HttpRequest &request, Client &client)
{
	return (request.getHeader("Content-Length").length() &&
			client.getServerBlock()->getDirectiveValue("client_max_body_size").length() &&
			std::stoi(request.getHeader("Content-Length")) > 1000000 *
			std::stoi(client.getServerBlock()->getDirectiveValue("client_max_body_size")));		
}

void	Server::processRequest(Client &client)
{
	if (!client.hasRequest())
		return;
	string			&req = client.getRequest();
	HttpResponse	*res = NULL;

	if (isHttpRequest(req))
	{
		HttpRequest	request(client);
		if (!isMethodAllowedInUploads(request, client)) //CHANGE NAME!
			res = new HttpResponse(405, "Method " + request.getMethod() + " not allowed.", request);
		else if (checkRequestSize(request, client))
			res =  new HttpResponse(413, "Payload is too large.", request);
		else if (request.getMethod() == "GET")
			res = new HttpResponse(handleGet(request, client));
		// else if (request.getMethod() == "POST")
		// else if (request.getMethod() == "DELETE")
		else
			res = new HttpResponse(501, "Method not implemented", request);
	}
	else
	{
		if (handleFileContent(client, req))
			res = new HttpResponse(500, "Upload Failed", HttpRequest(client));
	}
	if (res)
	{
		client.queueResponse(res->returnResponse());
	}

    this->postEvent(client.getSocket(), 2);
    client.popRequest();

	delete res;
}

HttpResponse	Server::retrieveFile(HttpRequest &request)
{
	string uri = request.getUri();
	const ServerBlock serverBlock = request.getRequestBlock();
	string root = serverBlock.getLocationValue(request.getMatched_location(), "root");
	string indexFile = serverBlock.getLocationValue(request.getMatched_location(), "index");
	string autoindex = serverBlock.getLocationValue(request.getMatched_location(), "autoindex");
	string fullPath = root + uri;

	if (std::filesystem::is_directory(fullPath))
	{
		std::string indexFilePath = fullPath + "/" + indexFile;
		if (!indexFile.empty() && std::filesystem::exists(indexFilePath) && std::filesystem::is_regular_file(indexFilePath))
		{
			std::string fileContent = Server::readFile(indexFilePath);
			std::string mimeType = Server::getMimeType(indexFilePath);

			//SET content-type header to mimetype
			return HttpResponse(200, fileContent, request);
		}

		if (autoindex == "on")
		{
			std::string html = "<html><head><title>Index of " + uri + "</title></head><body>";
			html += "<h1>Index of " + uri + "</h1><hr><pre>";

			for (const auto &entry : std::filesystem::directory_iterator(fullPath))
			{
				std::string name = entry.path().filename().string();
				std::string link = uri + (uri.back() == '/' ? "" : "/") + name;
				html += "<a href=\"" + link + "\">" + name + "</a>\n";
			}
			html += "</pre><hr></body></html>";
			//SET content-type header to text/html
			return HttpResponse(200, html, request);
		
		}
		return HttpResponse(403, "403 Forbidden", request);
	}

	if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath))
	{
		std::string fileContent = readFile(fullPath);
		std::string mimeType = getMimeType(fullPath);

		//SET content-type header to mimetype
		return HttpResponse(200, fileContent, request);
	}
	return HttpResponse(404, "404 not found", request);
}

HttpResponse	Server::handleGet(HttpRequest &request, Client &client)
{
	if (request.getUri() == "/list-uploads")
	{
		string jsonList = listUploadsJSON("./" + client.getServerBlock()->getLocationValue(request.getHeader("X-uploadEndpoint"), "root"), request.getHeader("X-uploadEndpoint"));
		return HttpResponse(200, jsonList, request);
	}
	else if (request.getUri() == "/return")
	{
		string redirect = client.getServerBlock()->getLocationValue("/return", "return");
		if (redirect.empty())
			return HttpResponse(500, "No reirection target", request);
		request.setRedirLocation(redirect);
		return HttpResponse(301, "", request);
	}
	else
		return retrieveFile(request);
}

int	Server::handleFileContent(Client &client, string &req)
{
	debug("FILE CONTENT");
	string boundaryPrefix = "--" + client.get_boundary() + "\r\n";
	string boundarySuffix = "--" + client.get_boundary() + "--\r\n";
	string *endBoundary;

	if (req.substr(req.length() - boundaryPrefix.length()) == boundaryPrefix)
		endBoundary = &boundaryPrefix;
	else if (req.substr(req.length() - boundarySuffix.length()) == boundarySuffix)
	{
		// debug("LAST CHUNK RECEIVED");
		endBoundary = &boundarySuffix;
	}
	else
	{
		endBoundary = NULL;
	}

	if (req.substr(0, boundaryPrefix.length()) != boundaryPrefix)
	{
		// debug("Missing Boundary prefix: " + boundaryPrefix);
		endBoundary = NULL;
	}

	if (endBoundary)
	{
		string content = req.substr(boundaryPrefix.length(), req.length());
		content = content.substr(0, content.length() - endBoundary->length());
		content = content.substr(0, content.length() - 2);

		size_t headerEndPos = content.find("\r\n\r\n");
		if (headerEndPos != string::npos)
			content = content.substr(headerEndPos + 4);
		client.queueFileContent(content);
		if (*endBoundary == boundarySuffix)
			postEvent(client.getSocket(), 4);
		else
			postEvent(client.getSocket(), 3);
	}
	else
	{
		client.isSending() = false;
		return -1;
	}
	return 0;
}

