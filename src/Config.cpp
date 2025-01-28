#include "../include/Webserv.hpp"
#include "../include/Config.hpp"
#include "../include/ServerBlock.hpp"

Config::Config(const std::string& conf_path) : _config_path(conf_path)
{
	parseConfig(); // Call parseConfig during construction
	if (_server_blocks.empty()) {
		/*UNCAUGHT*/ throw std::runtime_error("No server blocks found in configuration.");
	}
	isValid();
}

Config::~Config() {}


void Config::parseConfig() {
	std::ifstream config_file(_config_path);
	if (!config_file.is_open()) {
		/*UNCAUGHT*/ throw std::runtime_error("Failed to open file: " + _config_path);
	}

	std::string line;
	while (std::getline(config_file, line)) {
		// Check if the line marks the start of a server block
		if (line.find("server") != std::string::npos && line.find("{") != std::string::npos)
		{
			ServerBlock one_block(config_file);
			_server_blocks.push_back(one_block);
		}
	}
	config_file.close();
}

std::vector<ServerBlock>& Config::getServerBlocks() {
	return _server_blocks;
}

void Config::configDebug() const {
	for (const auto& block : _server_blocks) {
		std::cout << "\n------===== SERVER BLOCK =====------\n";

		std::cout << "\n===== HOST NAME =====\n";
		std::cout << "Host Name: " << block.getHostName() << "\n";
		std::cout << "\n===== PORT =====\n";
		std::cout << "Port: " << block.getPort() << "\n";
		std::cout << "\n===== DIRECTIVE PAIRS =====\n";
		for (const auto& pair : block.getDirectivePairs()) {
			std::cout << "Key: --" << pair.first << "<-- Value: -->" << pair.second << "<--\n";
		}

		std::cout << "\n===== ERROR PAGES =====\n";
		for (const auto& pair : block.getErrorPages()) {
			std::cout << "Key: -->" << pair.first << "<-- Value: -->" << pair.second << "<--\n";
		}

		std::cout << "\n===== LOCATION BLOCKS =====\n";
		for (const auto& location : block.getAllLocationBlocks()) {
			std::cout << "Location: -->" << location.first << "<--\n";
			for (const auto& location_pair : location.second) {
				std::cout << "Key: -->" << location_pair.first << "<-- Value: -->" << location_pair.second << "<--\n";
			}
		}
	}
}

void	Config::isValid() const
{
	int				open_braces = 0;

	std::ifstream config_file(_config_path);
	if (!config_file.is_open()) {
		/*UNCAUGHT*/ throw std::runtime_error("Failed to open file: " + _config_path);
	}

	std::string line;
	while (std::getline(config_file, line)) 
	{
		if (line.find("{") != std::string::npos)
			open_braces ++;
		if (line.find("}") != std::string::npos)
			open_braces--;
		if (open_braces < 0)
			/*UNCAUGHT*/ throw std::runtime_error("Config file braces do not match");
	}
	config_file.close();
	if (open_braces != 0)
		/*UNCAUGHT*/ throw std::runtime_error("config file braces do not match");

	int				port_val;
	std::string		host_val;


	for (size_t block = 0; block < _server_blocks.size(); block++)
	{
		port_val = _server_blocks[block].getPort();
		host_val = _server_blocks[block].getHostName();

		if (port_val == -1 || host_val.empty())
			/*UNCAUGHT*/ throw std::runtime_error("All server blocks must contain both 'listen' and 'server_name' directives");
		for (size_t next_block = block + 1; next_block < _server_blocks.size(); next_block++)
		{
			if (_server_blocks[next_block].getPort() == port_val && _server_blocks[next_block].getHostName() == host_val)
				/*UNCAUGHT*/ throw std::runtime_error("No 2 server blocks can have the same 'listen' and 'server_name' values. listen : " + std::to_string(port_val) + " server_name: " + host_val);
		}
	}
}
