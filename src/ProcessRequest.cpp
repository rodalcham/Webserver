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

bool isHttpRequest(const std::string &request);

void	Server::processRequest(Client &client)
{
	if (!client.hasRequest())
		return;
	string&	req = client.getRequest();
	HttpResponse *res;
	if (isHttpRequest(req))
	{
		HttpRequest(client);
	}
	else
	{
		handleFileContent();
	}
}