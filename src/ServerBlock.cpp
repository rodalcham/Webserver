#include "../include/Webserv.hpp"
#include "../include/ServerBlock.hpp"

ServerBlock::ServerBlock(std::ifstream& config_stream)
{
	parseBlock(config_stream);
}

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
		{
			std::cout << "\nlocation block\n";
			while (std::getline(stream, line))
			{
				if (line.find("}") != std::string::npos)
					continue;
			}
			continue;
		}
		// 	setLocationBlock(stream, line.substr(9));

		if (line.empty() || line[0] == '#')
			continue;
		if (line.find("{") != std::string::npos)
			continue;
		if (line.find("}") != std::string::npos)
			break;

		std::string directive = createDirectiveStr(line);

		size_t space_pos = directive.find(" ");
		if (space_pos == std::string::npos)
			throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

		std::string key = directive.substr(0, space_pos);
		std::string value = directive.substr(space_pos + 1);

		if (key == "server_name" || key == "listen" || key == "root" || key == "index")
			directive_pairs.insert({key, value});
		else if (key == "error_page")
			setErrorPage(value, line);			
		else
			throw std::runtime_error("Config file error: Unknown directive: " + key);
	}
}

std::string	ServerBlock::createDirectiveStr(std::string& line)
{
	size_t semicolon_pos = line.find(";");
	if (semicolon_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing semicolon in line: " + line);

	std::string directive = line.substr(0, semicolon_pos);
	directive.erase(0, directive.find_first_not_of(" \t")); //can be deleted??????????????????
	directive.erase(directive.find_last_not_of(" \t") + 1); //can be deleted??????????????????
	return (directive);
}

void	ServerBlock::setErrorPage(std::string& error_directive, std::string& line)
{
	size_t space_pos = error_directive.find(" ");
	if (space_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

	std::string key = error_directive.substr(0, space_pos);
	std::string value = error_directive.substr(space_pos + 1);

	error_pages.insert({key, value});
}

// void	ServerBlock::setLocationBlock(std::istream& stream, std::string line)
// {

// }



// void ServerBlock::debug()
// {

// }

