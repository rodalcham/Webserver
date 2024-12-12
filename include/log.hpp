#pragma once

#include "Webserv.hpp"

/**
 * A class used to log events into a file for later analysis
 * 
 * @param _log_file_path the path into the file to log, created by the class
 * @param _file The file fd
 */
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
