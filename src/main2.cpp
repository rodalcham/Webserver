#include "../include/Webserv.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/Config.hpp"
#include "../include/ServerBlock.hpp"

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file_path>\n";
		return 1;
	}

	Config config(argv[1]); // Load and validate configuration

	config.debug();

std::string request_string = 
    "GET /uploads/indsex.html HTTP/1.1\r\n"
    "Host: localhost:8081\r\n"
    "User-Agent: MyCustomBrowser/1.0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
    "Connection: keep-alive\r\n"
    "\r\n";

	std::vector<ServerBlock>	server_blocks = config.getServerBlocks();

	HttpRequest		request(request_string, server_blocks);
	HttpResponse	response(request);

	response.debug();

	return 0;
}

// c++ -Wall -Wextra -Werror -std=c++17 src/main2.cpp src/HTTPResponse.cpp src/HTTPRequest.cpp src/Config.cpp src/ServerBlock.cpp