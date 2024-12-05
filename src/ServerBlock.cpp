/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gstronge <gstronge@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:55 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 13:00:12 by gstronge         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
			setLocationBlock(stream, line.substr(9));

		if (line.empty() || line[0] == '#')
			continue;
		if (line.find("{") != std::string::npos)
			continue;
		if (line.find("}") != std::string::npos)
			break;

		std::string directive = createDirectiveStr(line);

		size_t space_pos = directive.find_first_of(" \t");
		if (space_pos == std::string::npos)
			throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

		std::string key = directive.substr(0, space_pos);
		std::string value = directive.erase(0, directive.find_first_not_of(" \t", space_pos));


		if (key == "server_name" || key == "listen" || key == "root" || key == "index" || key == "client_max_body_size")
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
	directive.erase(0, directive.find_first_not_of(" \t"));
	directive.erase(directive.find_last_not_of(" \t") + 1);
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

void	ServerBlock::setLocationBlock(std::istream& stream, std::string line)
{
	size_t bracket_pos = line.find("{");
	if (bracket_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing { in line: " + line);

	std::string location = line.substr(0, bracket_pos);
	location.erase(0, location.find_first_not_of(" \t"));
	location.erase(location.find_last_not_of(" \t") + 1);

	while (std::getline(stream, line))
	{
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		if (line.empty() || line[0] == '#')
			continue;
		if (line.find("{") != std::string::npos)
			continue;
		if (line.find("}") != std::string::npos)
			break;

		std::string directive = createDirectiveStr(line);

		size_t space_pos = directive.find_first_of(" \t");
		if (space_pos == std::string::npos)
			throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

		std::string key = directive.substr(0, space_pos);
		std::string value = directive.erase(0, directive.find_first_not_of(" \t", space_pos));

		if (key == "allow_methods" || key == "index" || key == "root" || key == "autoindex" || key == "cgi_pass" || key == "return" || key == "client_max_body_size")
			location_blocks[location].insert({key, value});
		// else if (key == "error_page")
		// 	setErrorPage(value, line);			
		else
			throw std::runtime_error("Config file error: Unknown directive: " + key);
	}
}



// void ServerBlock::debug()
// {

// }

