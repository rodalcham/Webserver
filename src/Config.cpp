#include "../include/Webserv.hpp"
#include "../include/Config.hpp"
#include "../include/ServerBlock.hpp"

Config::Config(const std::string& conf_path) : config_path(conf_path)
{
	std::ifstream config_file(config_path);
	
	if (!config_file.is_open())
		throw std::runtime_error("Failed to open file: " + config_path);

	ServerBlock one_block(config_file);
	debug(one_block);
	// while (config_file)
	// {
	// 	server_blocks.push_back(ServerBlock(config_file));
	// }
}

Config::~Config() {}

void	Config::debug(ServerBlock one_block)
{
	std::cout << "\n ===== DIRECTIVE PAIRS =====\n\n";
	for (const auto& pair : one_block.directive_pairs)
		std::cout << "Key: --" << pair.first << "<-- Value: -->" << pair.second << "<--\n";
	std::cout << "\n ===== ERROR PAGES =====\n\n";
	for (const auto& pair : one_block.error_pages)
		std::cout << "Key: -->" << pair.first << "<-- Value: -->" << pair.second << "<--\n";
	std::cout << "\n ===== LOCATION BLOCK =====\n";
	for (const auto& pair : one_block.location_blocks)
	{
		std::cout << "\nLocation: -->" << pair.first << "<--\n";
		for (const auto& location_pair : pair.second)
			std::cout << "Key: -->" << location_pair.first << "<-- Value: -->" << location_pair.second << "<--\n";
	}
}
