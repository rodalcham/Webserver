/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchavez <rchavez@student.42heilbronn.de    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 10:29:43 by rchavez           #+#    #+#             */
/*   Updated: 2024/12/05 12:16:09 by rchavez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Webserv.hpp"
#include "../include/Server.hpp"
#include "../include/Config.hpp"

void signalHandler(int signum) {
	std::cout << "\nInterrupt signal (" << signum << ") received.\n";
	keepRunning = false; // Stop the server loop
}

void setupSignalHandler() {
	struct sigaction action;
	action.sa_handler = signalHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction(SIGINT, &action, nullptr) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}


int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file_path>\n";
		return 1;
	}
	debug("Server running on DEBUG mode.");
	try
	{
		Config config_obj(argv[1]);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	setupSignalHandler(); // Set up the signal handler
	try {
		Server server; // Create server instance
		while (keepRunning) { // Main loop
			server.run();
		}
		std::cout << "Server shutting down...\n";
	} catch (const std::exception &e) {
		std::cerr << "Server error: " << e.what() << std::endl;
	}
	return 0;
}
