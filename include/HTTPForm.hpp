/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPForm.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:27 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 10:29:28 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

class HTTPForm
{
protected:
	
	string	process;
	string	headers;
	string	body;
	int		status;

public:

	HTTPForm(/* args */);
	~HTTPForm();

};
