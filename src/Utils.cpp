/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:43:44 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 10:49:50 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Webserv.hpp"

uint16_t	ft_htons(uint16_t port)
{
    uint16_t test = 1;

    bool isLittleEndian = *(reinterpret_cast<uint8_t*>(&test)) == 1;

    if (isLittleEndian) {
        return (port >> 8) | (port << 8);
    } else {
        return port;
    }
}

void	debug(string message)
{
	if	(DEBUG)
		cout << "\n" << message << "\n";
}