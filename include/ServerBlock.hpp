/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:34 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 17:58:43 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"
#include <string>
#include <map>

class ServerBlock
{
public:
	std::map<std::string, std::string>							directive_pairs;
	std::map<std::string, std::string>							error_pages;
	std::map<std::string, std::map<std::string, std::string>>	location_blocks;

	// ServerBlock();
	ServerBlock(std::ifstream& config_stream);
	~ServerBlock();

	void		parseBlock(std::istream& stream);
	void		setErrorPage(std::string& value, std::string& line);
	void		setDirective(std::string& key, std::string& value);
	std::string	createDirectiveStr(std::string& line);
	void		setLocationBlock(std::istream& stream, std::string line);
};

	// void		debug();