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
bool	isCGIRequest(const HttpRequest &request);
string	listUploadsJSON(const std::string &, string endpoint);
string	resolveCGIPath(const std::string &uri);

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
	HttpResponse	res;

	if (isHttpRequest(req))
	{
		HttpRequest	request(client);
		if (!isMethodAllowedInUploads(request, client)) //CHANGE NAME!
			res = HttpResponse(405, "Method " + request.getMethod() + " not allowed.", request);
		else if (checkRequestSize(request, client))
			res = HttpResponse(413, "Payload is too large.", request);
		else if (request.getMethod() == "GET")
			res = handleGet(request, client);
		else if (request.getMethod() == "POST")
		{
			if (isCGIRequest(request))
			{
				if (handleCGI(request, &client, req))
					res = HttpResponse(500, "Failed to launch child process", HttpRequest(client));
			}
			else if (handlePost(request, client))
				res = HttpResponse(500, "Upload Failed", HttpRequest(client));
		}
		else if (request.getMethod() == "DELETE")
			res = handleDelete(request, client);
		else
			res = HttpResponse(501, "Method not implemented", request);
	}
	else
	{
		if (handleFileContent(client, req))
			res = HttpResponse(500, "Upload Failed", HttpRequest(client));
	}
	if (res.isReady())
	{
		client.queueResponse(res.returnResponse());
		postEvent(client.getSocket(), 2);
		client.popRequest();
	}
}

int	Server::handleCGI(HttpRequest &request, Client *client, string &req)
{
	string path = resolveCGIPath(request.getUri());
	int	cgiOutput[2];
	if (pipe(cgiOutput) < 0)
		return -1;
	client->setPid(fork());
	if (client->getPid() < 0)
	{
		close(cgiOutput[0]);
		close(cgiOutput[1]);
		return -1;
	}
	else if (client->getPid() == 0)
	{
		close(cgiOutput[0]);
		dup2(cgiOutput[1], STDOUT_FILENO);
		close(cgiOutput[1]);

		string::size_type dotPos = path.find_last_of('.');
		string extension;

		if (dotPos != std::string::npos)
			extension = path.substr(dotPos + 1); // "py", "php", etc.

		const char* interpreter = "/usr/bin/python3";
		if (extension == "php")
			interpreter = "/usr/bin/php";
		else if (extension == "py")
			interpreter = "/usr/bin/python3";

		char* const args[] = {
			const_cast<char*>(interpreter),
			const_cast<char*>(path.c_str()),
			const_cast<char*>(req.c_str()),  // optional if your script reads from argv
			nullptr
		};
		if (execve(interpreter, args, nullptr) == -1)
		{
			_exit(-1);
		}
		else
		{
			close(cgiOutput[1]);
			client->setCGIOutput(cgiOutput[0]);
			struct kevent event;
			EV_SET(&event, cgiOutput[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, client);
			if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
			{
				close(cgiOutput[0]);
				return -1;
			}
		}
	}
}

int	Server::handlePost(HttpRequest &request, Client &client)
{
	string filename = request.getHeader("filename");
	string uri = request.getUri();
	string root = "./" + client.getServerBlock()->getLocationValue(uri, "root");
	if (filename.empty())
		return -1;
	client.get_outFile().open(root + uri + filename, std::ios::binary);
	if (!client.get_outFile().is_open())
		return -1;
	client.isSending() = true;
	return 0;
}

HttpResponse	Server::handleDelete(HttpRequest &request, Client &client)
{
	string uri = request.getUri();
	if (uri.rfind(request.getMatched_location(), 0) == 0)
	{
		std::string filename = uri.substr(std::string(uri).size());
		std::string fullPath = "./" + client.getServerBlock()->getLocationValue(uri, "root") + uri + filename;

		if (std::remove(fullPath.c_str()) == 0)
			return HttpResponse(200, "File deleted Successfully", request);
		else
			return HttpResponse(404, "File not found or cannot delete", request);
	}
	else
		return HttpResponse(400, "Invalid Delete path", request);
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
		endBoundary = &boundarySuffix;
	}
	else
	{
		endBoundary = NULL;
	}

	if (req.substr(0, boundaryPrefix.length()) != boundaryPrefix)
	{
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

