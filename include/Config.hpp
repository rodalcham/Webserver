#pragma once

#include "Webserv.hpp"
#include "ServerBlock.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

class Config {
private:
    const std::string           config_path;
    std::vector<ServerBlock>    server_blocks;

    void parseConfig(); // Private method to parse the config file

public:
    Config(const std::string& conf_path);
    ~Config();

    const std::vector<ServerBlock>& getServerBlocks() const;
    void debug() const; // Debug all server blocks
};
