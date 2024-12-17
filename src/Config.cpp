#include "../include/Webserv.hpp"
#include "../include/Config.hpp"
#include "../include/ServerBlock.hpp"

Config::Config(const std::string& conf_path) : _config_path(conf_path) {
	parseConfig(); // Call parseConfig during construction
	if (_server_blocks.empty()) {
		throw std::runtime_error("No server blocks found in configuration.");
	}
}

Config::~Config() {}

void Config::parseConfig() {
	std::ifstream config_file(_config_path);
	if (!config_file.is_open()) {
		throw std::runtime_error("Failed to open file: " + _config_path);
	}

	std::string line;
	while (std::getline(config_file, line)) {
		// Check if the line marks the start of a server block
		if (line.find("server") != std::string::npos && line.find("{") != std::string::npos) {
			ServerBlock one_block(config_file);
			_server_blocks.push_back(one_block);
		}
	}
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
