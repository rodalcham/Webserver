#pragma once

#include "Webserv.hpp"
#include <string>
#include <map>

class HttpRequest;


class ServerBlock
{
private:
	int															_port = -1;
	std::string													_host_name;
	int															_socket_no;
	long long													_max_body_size = 1000000;
	std::map<std::string, std::string>							_directive_pairs;
	std::map<std::string, std::string>							_error_pages;
	std::map<std::string, std::map<std::string, std::string>>	_location_blocks;

public:
	ServerBlock(std::ifstream& config_stream);
	~ServerBlock();

	void			parseBlock(std::istream& stream);
	void			setErrorPage(std::string& value, std::string& line);
	void			setRedirect(std::string& value, std::string& line);
	void			setMaxBodySize(std::string value);
	std::string		createDirectiveStr(std::string& line);
	void			serverBlockDebug() const;
	bool			duplicateServers() const;

	// SETTERS
	void			setSocketNo(const int& socket_number);
	void			setLocationBlock(std::istream& stream, std::string line);

	// GETTERS
	std::string													getHostName() const;
	int															getPort() const;
	int															getSocketNo() const;
	int															getMaxBodySize() const;
	std::map<std::string, std::string>							getDirectivePairs() const;
	std::map<std::string, std::string>							getErrorPages() const;
	std::map<std::string, std::map<std::string, std::string>>	getAllLocationBlocks() const;
	std::string													getDirectiveValue(std::string key) const;
	std::string													getErrorPageValue(std::string key) const;
	std::map<std::string, std::string>							getLocationBlock(std::string location) const;
	std::string													getLocationValue(std::string location, std::string key) const;
};

