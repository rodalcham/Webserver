#pragma once

#include "Webserv.hpp"

class ServerBlock;

/**
 * A class for ?
 */
class Config
{
public:
	const std::string			config_path;
	// std::vector<ServerBlock>	server_blocks;

	Config(const std::string& conf_path);
	~Config();

	void	debug(ServerBlock one_block);
};

	// std::vector<ServerBlock>	server_blocks;