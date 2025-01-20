#include "../include/Webserv.hpp"
#include "../include/log.hpp"

Log::Log()
{
	std::string log_dir = "./logs/";
	if (!std::filesystem::exists(log_dir))
	{
		if (!std::filesystem::create_directory(log_dir))
			throw std::runtime_error("Failed to create log directory: " + log_dir);
	}
	_log_file_path = log_dir + getTimestamp() + "_logfile.txt";
	_file.open(_log_file_path);

	if (!_file)
	{
		throw std::runtime_error("Failed to open log file: " + _log_file_path);
	}
	
}

Log::~Log()
{
	_file.close();
}

void	Log::event(const std::string& message)
{
	std::string	timestamp;

	timestamp = "[" + getTimestamp() + "] ";
	_file << timestamp << message << std::endl;
}

std::string	Log::getTimestamp()
{
	std::time_t time_since_epoch = std::time(nullptr);
	std::tm* current_time = std::localtime(&time_since_epoch);
	std::ostringstream timestamp_stream;

	timestamp_stream << std::put_time(current_time, "%Y%m%d") 
					<< "_" << std::put_time(current_time, "%H%M%S");

	return (timestamp_stream.str());
}
