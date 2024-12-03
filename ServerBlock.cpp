#include "Webserv.hpp"
#include "ServerBlock.hpp"

ServerBlock::ServerBlock()
{



}

// ServerBlock::ServerBlock(const int& conf_fd) : listen_port(8080)
// {

// }

ServerBlock::~ServerBlock()
{

}

void ServerBlock::parseBlock(std::istream& stream)
{
	std::string line;

	while (std::getline(stream, line))
	{
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		if (line.substr(0, 9) == "location ")
			setLocationBlock(stream, line);
		if (line.empty() || line[0] == '#')
			continue;
		if (line.find("{") != std::string::npos) // add strncmp or something similar to exclude if the first word is location
			continue;
		if (line.find("}") != std::string::npos) // add strncmp or something similar to exclude if the first word is location
			break;

		size_t delimiter_pos = line.find(";"); // add strncmp or something similar to exclude if the first word is location
		if (delimiter_pos == std::string::npos)
			throw std::runtime_error("Syntax error: Missing semicolon in line: " + line);

		std::string directive = line.substr(0, delimiter_pos);
		directive.erase(0, directive.find_first_not_of(" \t"));
		directive.erase(directive.find_last_not_of(" \t") + 1);

		size_t space_pos = directive.find(" ");
		if (space_pos == std::string::npos)
			throw std::runtime_error("Syntax error: Missing value for directive in line: " + line);

		std::string key = directive.substr(0, space_pos);
		std::string value = directive.substr(space_pos + 1);

		if (key == "server_name" || key == "listen" || key == "root" || key == "index")
			setDirective();
		else if (key == "error_page")
			setErrorPage(value);			
		else
			throw std::runtime_error("Unknown directive: " + key);
	}
}

void	ServerBlock::setErrorPage(std::string& value)
{
	std::istringstream value_stream(value);
	std::string error_code;
	std::string file_path;

	// Extract the error code and file path from the value
	if (value_stream >> error_code >> file_path)
	{
		// Ensure the error code is a valid integer-like string
		try {
			std::stoi(error_code);  // This will throw if it's not a valid number
		} catch (const std::invalid_argument&) {
			throw std::runtime_error("Invalid error code in error_page directive: " + error_code);
		}

		// Store the error page for the respective error code
		error_pages[error_code] = file_path;
	}
	else
		throw std::runtime_error("Invalid error_page directive format: " + value);
}

void	ServerBlock::setLocationBlock()
{

}

void	ServerBlock::setErrorPage(std::string& value)
{
	std::istringstream value_stream(value);
	std::string error_code;
	std::string file_path;

	// Extract the error code and file path from the value
	if (value_stream >> error_code >> file_path)
	{
		// Ensure the error code is a valid integer-like string
		try {
			std::stoi(error_code);  // This will throw if it's not a valid number
		} catch (const std::invalid_argument&) {
			throw std::runtime_error("Invalid error code in error_page directive: " + error_code);
		}

		// Store the error page for the respective error code
		error_pages[error_code] = file_path;
	}
	else
		throw std::runtime_error("Invalid error_page directive format: " + value);
}

void ServerBlock::debug()
{
	std::cout << "\n";
	std::cout << "Server Name: " << server_name << "\n";
	std::cout << "Listen Port: " << listen_port << "\n";
	std::cout << "Root Directory: " << server_block.root_dir << "\n";
	std::cout << "Index File: " << server_block.index << "\n";

	std::cout << "Error Pages:\n";
	for (const auto& error_page : server_block.error_pages)
	{
		std::cout << "  Error " << error_page.first << ": " << error_page.second << "\n";
	}
}

