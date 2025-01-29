#pragma once

#include "Webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

#include <fstream>
#include <sstream>


class GetResponse : public HttpResponse
{
public:
	GetResponse();
	GetResponse(HttpRequest& request);
	~GetResponse();

	void		makeHeaderList(HttpRequest& request);
	void		parseBody();
};
