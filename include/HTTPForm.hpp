#pragma once

#include "Webserv.hpp"

class HTTPForm
{
protected:
	
	string	process;
	string	headers;
	string	body;
	int		status;

public:

	HTTPForm(/* args */);
	~HTTPForm();

};
