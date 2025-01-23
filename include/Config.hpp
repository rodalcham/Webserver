#pragma once

#include "Webserv.hpp"
#include "ServerBlock.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

/**
 * A class for ?
 */

class Config {
private:
    const std::string           _config_path;
    std::vector<ServerBlock>    _server_blocks;

    void parseConfig(); // Private method to parse the config file

public:
    Config(const std::string& conf_path);
    ~Config();

    std::vector<ServerBlock>&	getServerBlocks();
    void						configDebug() const; // Debug all server blocks
	void						isValid() const;
};
