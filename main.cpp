#include "Webserv.hpp"
// #include "Server.hpp"
// #include <csignal>
// #include <iostream>
// #include <atomic>
// #include <fstream>
// #include <sstream>
// #include <vector>
// #include <string>

bool keepRunning = true; // Initialize as needed

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

int main() {
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
