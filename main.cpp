/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbankhar <mbankhar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/27 15:03:13 by rchavez           #+#    #+#             */
/*   Updated: 2024/11/28 10:55:32 by mbankhar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Server.hpp"
#include <csignal>
#include <iostream>
#include <atomic>

// Global flag to control the server's main loop
std::atomic<bool> keepRunning(true);

// Signal handler to capture SIGINT (Ctrl+C)
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    keepRunning = false; // Set the flag to stop the server loop
}

// Set up signal handling
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

// Main function
int main() {
    setupSignalHandler(); // Set up the signal handler
    try {
        Server server; // Create the server instance
        server.run();  // Start the server loop
    } catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    return 0;
}
