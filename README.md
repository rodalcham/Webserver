# Webserv Project

## Overview
This project is a custom HTTP web server written in C++, inspired by the NGINX web server. It is designed to handle HTTP requests, serve static files, and support CGI execution. It provides basic web server functionalities including GET, POST, and DELETE methods, error handling, and configurable server blocks.

Project done in collaboration with [Máté Bankhardt](https://github.com/mbankhar) and [Graham Stronge](https://github.com/gstronge)

## Features
- Serve static files (HTML, CSS, JS, images, etc.)
- Support for CGI scripts (Python, PHP)
- Customizable server configuration through configuration files
- Error handling with customizable error pages
- Multiple server block support with directives like `listen`, `server_name`, `root`, `index`, and more
- Autoindexing for directory listing
- Connection timeout management

## Getting Started

### Prerequisites
- C++17 compatible compiler (e.g., g++)
- Makefile utility
- POSIX-compliant operating system (Linux, macOS)

### Building the Project
```bash
make
```
This command compiles the project and creates the executable `webserv`.

### Running the Server
```bash
./webserv <path_to_config_file>
```
Example:
```bash
./webserv ./config/config_TEMPLATE.conf
```

### Stopping the Server
Press `Ctrl + C` to gracefully shut down the server.

## Configuration
The server configuration is managed through a configuration file. Each server block can have directives like:
- `listen`: Port number to listen on
- `server_name`: Hostname of the server
- `root`: Directory to serve files from
- `index`: Default file to serve when a directory is accessed
- `error_page`: Custom error pages for different HTTP status codes
- `client_max_body_size`: Maximum allowed body size for client requests

### Sample Configuration
```nginx
server {
    listen 8080;
    server_name localhost;

    root /var/www/html;
    index index.html;

    location / {
        autoindex on;
        allow_methods GET POST DELETE;
    }

    error_page 404 /404.html;
}
```

## Supported HTTP Methods
- GET: Retrieve resources from the server
- POST: Submit data to the server (file upload supported)
- DELETE: Remove resources from the server

## Error Handling
Custom error pages can be defined for different HTTP status codes using the `error_page` directive in the configuration file.

## Autoindex
When enabled, the server generates a directory listing for folders that do not have an `index` file.

## CGI Support
Supports executing CGI scripts written in Python and PHP. Ensure the script has execution permissions and the appropriate shebang line (e.g., `#!/usr/bin/python3` for Python).

## Logging and Debugging
Logs and debug messages are generated based on the DEBUG flag in the source code.

