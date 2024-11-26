/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPForm.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/26 10:50:20 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/26 11:44:47 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

typedef class HTTPForm
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
