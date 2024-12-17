#pragma once

#include "Webserv.hpp"
#include <string>
#include <map>

class HttpRequest;

/**
 * A class to store the config block for each server
 * 
 * @param directive_pairs ?
 * @param error_pages ?
 * @param location_blocks ?
 */

class ServerBlock
{
private:
	int															_port;
	std::string													_host_name;
	std::map<std::string, std::string>							_directive_pairs;
	std::map<std::string, std::string>							_error_pages;
	std::map<std::string, std::map<std::string, std::string>>	_location_blocks;

public:
	// ServerBlock();
	ServerBlock(std::ifstream& config_stream);
	~ServerBlock();

	void			parseBlock(std::istream& stream);
	void			setErrorPage(std::string& value, std::string& line);
	// void			setDirective(std::string& key, std::string& value);
	std::string		createDirectiveStr(std::string& line);
	void			setLocationBlock(std::istream& stream, std::string line);
	void			serverBlockDebug() const;
	bool			isRequestAllowed(const HttpRequest& request) const;

	// SETTERS
	std::string													getHostName() const;
	int															getPort() const;
	std::map<std::string, std::string>							getDirectivePairs() const;
	std::map<std::string, std::string>							getErrorPages() const;
	std::map<std::string, std::map<std::string, std::string>>	getAllLocationBlocks() const;
	std::string													getDirectiveValue(std::string key) const;
	std::string													getErrorPageValue(std::string key) const;
	std::map<std::string, std::string>							getLocationBlock(std::string location) const;
	std::string													getLocationValue(std::string location, std::string key) const;
};

