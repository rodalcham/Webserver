/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:22 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 10:29:26 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

class ServerBlock;

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