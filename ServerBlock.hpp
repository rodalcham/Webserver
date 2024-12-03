#pragma once

#include "Webserv.hpp"

class ServerBlock
{
public:
	std::ifstream												config_file;
	std::map<std::string, std::string>							directive_pairs;
	std::map<std::string, std::string>							error_pages;
	std::map<std::string, std::map<std::string, std::string>>	location_blocks;

	ServerBlock();
	// ServerBlock(std::ifstream& conf_fd);
	~ServerBlock();

	void	parseBlock(std::istream& stream);
	void	setErrorPage(std::string& value);
	void	setLocationBlock(std::istream& stream, std::string& line);
	void	setDirective();
	void	debug();
};
