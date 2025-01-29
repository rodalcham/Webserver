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


		if (key == "listen")
			_port = std::stoi(value);
		else if (key == "server_name")
			_host_name = value;
		else if (key == "root" || key == "index" || key == "allow_methods" || key == "autoindex" || key == "cgi_pass")
			_directive_pairs.insert({key, value});
		else if (key == "error_page")
			setErrorPage(value, line);	
		else if (key == "return")
		{
			value.erase(0, value.find_first_of(" \t"));
			value.erase(0, value.find_first_not_of(" \t"));
			_directive_pairs.insert({key, value});
		}
		else if (key == "client_max_body_size")
			setMaxBodySize(value);
		else
			throw std::runtime_error("Config file error: Unknown directive: " + key);
	}
	_host_name = _host_name + ":" + std::to_string(_port);
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

void	ServerBlock::setRedirect(std::string& error_directive, std::string& line)
{
	size_t space_pos = error_directive.find(" ");
	if (space_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

	std::string key = error_directive.substr(0, space_pos);
	std::string value = error_directive.substr(space_pos + 1);

	_error_pages.insert({key, value});
}

void	ServerBlock::setErrorPage(std::string& error_directive, std::string& line)
{
	size_t space_pos = error_directive.find(" ");
	if (space_pos == std::string::npos)
		throw std::runtime_error("Config file error: Missing value for directive in line: " + line);

	std::string key = error_directive.substr(0, space_pos);
	std::string value = error_directive.substr(space_pos + 1);

	_error_pages.insert({key, value});
}

void ServerBlock::setLocationBlock(std::istream& stream, std::string line)
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

		if (key == "allow_methods" || key == "index" || key == "root" || key == "autoindex" || key == "cgi_pass" || key == "client_max_body_size")
			_location_blocks[location].insert({key, value});
		else if (key == "return")
		{
			value.erase(0, value.find_first_of(" \t"));
			value.erase(0, value.find_first_not_of(" \t"));
			_location_blocks[location].insert({key, value});
		}
		else if (key == "error_page")
		{
			std::istringstream iss(value);
			std::string error_code, error_path;
			iss >> error_code >> error_path;

			if (error_code.empty() || error_path.empty())
				throw std::runtime_error("Invalid error_page directive in line: " + line);

			_location_blocks[location].insert({"error_page_" + error_code, error_path});
		}
		else
			throw std::runtime_error("Config file error: Unknown directive: " + key);
	}
}


void ServerBlock::serverBlockDebug() const {
	std::cout << "ServerBlock Details:\n";

	std::cout << "\nDirective Pairs:\n";
	for (const auto& pair : _directive_pairs) {
		std::cout << "  " << pair.first << ": " << pair.second << "\n";
	}

	std::cout << "\nError Pages:\n";
	for (const auto& pair : _error_pages) {
		std::cout << "  " << pair.first << ": " << pair.second << "\n";
	}

	std::cout << "\nLocation Blocks:\n";
	for (const auto& block : _location_blocks) {
		std::cout << "  Location: " << block.first << "\n";
		for (const auto& directive : block.second) {
			std::cout << "    " << directive.first << ": " << directive.second << "\n";
		}
	}
}

void	ServerBlock::setSocketNo(const int& socket_number) {
	this->_socket_no = socket_number;
}

void	ServerBlock::setMaxBodySize(std::string value)
{
	if (value.empty())
		throw std::runtime_error("Config file error: client_max_body_size is empty");
	size_t pos = value.find_first_not_of("0123456789");
	if (pos != std::string::npos)
	{
		char		units = value.back();
		long long	num = std::stoi(value.substr(0, pos));
		if (num <= 0)
			throw std::runtime_error("Config file error: client_max_body_size: " + value);

		if (units == 'k' || units == 'K')
			_max_body_size = num * 1000;
		else if (units == 'm' || units == 'M')
			_max_body_size = num * 1000000;
		else if (units == 'g' || units == 'G')
			_max_body_size = num * 1000000000;
		else if (units == 't' || units == 'T')
			_max_body_size = num * 1000000000000;
		else
			throw std::runtime_error("Config file error: client_max_body_size: " + value);
	}
	else
		_max_body_size = std::stoi(value);
	if (_max_body_size < 0)
		throw std::runtime_error("Config file error: client_max_body_size outside limits: " + value);
}

std::string	ServerBlock::getHostName() const {
	return (_host_name);
}

int	ServerBlock::getPort() const {
	return (_port);
}

std::map<std::string, std::string>	ServerBlock::getDirectivePairs() const {
	return (_directive_pairs);
}

std::map<std::string, std::string>	ServerBlock::getErrorPages() const {
	return (_error_pages);
}

std::map<std::string, std::map<std::string, std::string>>	ServerBlock::getAllLocationBlocks() const {
	return (_location_blocks);
}

std::string	ServerBlock::getDirectiveValue(std::string key) const {
	for (const auto& pair : _directive_pairs) {
		if (pair.first == key)
			return (pair.second);
	}
	return ("");
}

std::string	ServerBlock::getErrorPageValue(std::string key) const {
	for (const auto& pair : _error_pages) {
		if (pair.first == key)
			return (pair.second);
	}
	return ("");
}

std::map<std::string, std::string>	ServerBlock::getLocationBlock(std::string location) const {
	for (const auto& loc : _location_blocks) {
		if ( loc.first == location) {
				return (loc.second);
		}
	}
	return std::map<std::string, std::string>();
}

std::string	ServerBlock::getLocationValue(std::string location, std::string key) const {
	for (const auto& loc : _location_blocks){
		if ( loc.first == location) {
			for (const auto& pair : loc.second) {
				if (pair.first == key)
				return (pair.second);
			}
		}
	}
	if (key == "index")
        return ("");
	return (getDirectiveValue(key));
}

int	ServerBlock::getSocketNo() const {
	return (_socket_no);
}

int	ServerBlock::getMaxBodySize() const {
	return (_max_body_size);
}
