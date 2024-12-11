#pragma once

#include "Webserv.hpp"

class Log
{
private:
	std::string		_log_file_path;
	std::ofstream	_file;

public:
	Log();
	~Log();

	void		event(const std::string& message);
	std::string	getTimestamp();
};
