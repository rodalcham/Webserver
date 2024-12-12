#include "../include/Webserv.hpp"
#include "../include/ServerBlock.hpp"

ServerBlock::ServerBlock(std::ifstream& config_stream)
{
	parseBlock(config_stream);
}

ServerBlock::~ServerBlock()
{

}

void ServerBlock::parseBlock(std::istream& stream)
{
	std::string line;

	while (std::getline(stream, line))
	{
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		if (line.substr(0, 9) == "location ")
			setLocationBlock(stream, line.substr(9));

		if (line.empty() || line[0] == '#')
			continue;
		if (line.find("{") != std::string::npos)
			continue;
		if (line.find("}") != std::string::npos)
			break;

		std::string directive = createDirectiveStr(line);

		size_t space_pos = directive.find_first_of(" \t");
		if (space_pos == std::string::npos)
			throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

		std::string key = directive.substr(0, space_pos);
		std::string value = directive.erase(0, directive.find_first_not_of(" \t", space_pos));


		if (key == "server_name" || key == "listen" || key == "root" || key == "index" || key == "client_max_body_size")
			directive_pairs.insert({key, value});
		else if (key == "error_page")
			setErrorPage(value, line);			
		else
			throw std::runtime_error("Config file error: Unknown directive: " + key);
	}
}

std::string	ServerBlock::createDirectiveStr(std::string& line)
{
	size_t semicolon_pos = line.find(";");
	if (semicolon_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing semicolon in line: " + line);

	std::string directive = line.substr(0, semicolon_pos);
	directive.erase(0, directive.find_first_not_of(" \t"));
	directive.erase(directive.find_last_not_of(" \t") + 1);
	return (directive);
}

void	ServerBlock::setErrorPage(std::string& error_directive, std::string& line)
{
	size_t space_pos = error_directive.find(" ");
	if (space_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

	std::string key = error_directive.substr(0, space_pos);
	std::string value = error_directive.substr(space_pos + 1);

	error_pages.insert({key, value});
}

void	ServerBlock::setLocationBlock(std::istream& stream, std::string line)
{
	size_t bracket_pos = line.find("{");
	if (bracket_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing { in line: " + line);

	std::string location = line.substr(0, bracket_pos);
	location.erase(0, location.find_first_not_of(" \t"));
	location.erase(location.find_last_not_of(" \t") + 1);

	while (std::getline(stream, line))
	{
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		if (line.empty() || line[0] == '#')
			continue;
		if (line.find("{") != std::string::npos)
			continue;
		if (line.find("}") != std::string::npos)
			break;

		std::string directive = createDirectiveStr(line);

		size_t space_pos = directive.find_first_of(" \t");
		if (space_pos == std::string::npos)
			throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

		std::string key = directive.substr(0, space_pos);
		std::string value = directive.erase(0, directive.find_first_not_of(" \t", space_pos));

		if (key == "allow_methods" || key == "index" || key == "root" || key == "autoindex" || key == "cgi_pass" || key == "return" || key == "client_max_body_size")
			location_blocks[location].insert({key, value});
		// else if (key == "error_page")
		// 	setErrorPage(value, line);			
		else
			throw std::runtime_error("Config file error: Unknown directive: " + key);
	}
}



void ServerBlock::debugPrint() const {
	std::cout << "ServerBlock Details:\n";

	std::cout << "\nDirective Pairs:\n";
	for (const auto& pair : directive_pairs) {
		std::cout << "  " << pair.first << ": " << pair.second << "\n";
	}

	std::cout << "\nError Pages:\n";
	for (const auto& pair : error_pages) {
		std::cout << "  " << pair.first << ": " << pair.second << "\n";
	}

	std::cout << "\nLocation Blocks:\n";
	for (const auto& block : location_blocks) {
		std::cout << "  Location: " << block.first << "\n";
		for (const auto& directive : block.second) {
			std::cout << "    " << directive.first << ": " << directive.second << "\n";
		}
	}
}

bool ServerBlock::isRequestAllowed(const HttpRequest& request) const {
    std::string uri = request.getUri();
    std::cerr << "[DEBUG] Checking if request URI: " << uri << " is allowed.\n";

    // Match URI against location blocks
    auto locationIt = location_blocks.end();
    for (auto it = location_blocks.begin(); it != location_blocks.end(); ++it) {
        if (uri.compare(0, it->first.length(), it->first) == 0) {
            locationIt = it; // Found matching location
            break;
        }
    }

    if (locationIt == location_blocks.end()) {
        std::cerr << "[DEBUG] No matching location block for URI: " << uri << "\n";
        return false;
    }

    const auto& locationDirectives = locationIt->second;

    // Check allowed methods
    auto methodIt = locationDirectives.find("allow_methods");
    if (methodIt != locationDirectives.end()) {
        std::string allowedMethods = methodIt->second;
        std::string requestMethod = request.getMethod();
        if (allowedMethods.find(requestMethod) == std::string::npos) {
            std::cerr << "[DEBUG] Request method not allowed: " << requestMethod << "\n";
            return false;
        }
    }

    // // Check file accessibility
    // std::string filePath = request.getFilePath();
    // if (!std::ifstream(filePath).good()) {
    //     std::cerr << "[DEBUG] File not accessible: " << filePath << "\n";
    //     return false;
    // }

    return true;
}
