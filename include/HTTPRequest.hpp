#pragma once
#include "Webserv.hpp"
#include "ServerBlock.hpp"
#include "Config.hpp"
#include <string>
#include <map>

class Client;
class HttpRequest
{
private:
	int 								_stat_code_no;
	int 								_port; // Can be removed if encapsulated in ServerBlock
	std::string 						_method;
	std::string 						_uri;
	std::string 						_http_version;
	std::map<std::string, std::string> 	_headers;
	std::string 						_body;
	std::string 						_filename;
	const ServerBlock* 					_request_block;
	std::string 						_continue_response; // Store the "100 Continue" response if needed
	std::string 						_file_content;

public:
	// CONSTRUCTORS
	HttpRequest();
	HttpRequest(Client &client);
	~HttpRequest();

	// GETTERS
	std::string 						getMethod() const;
	std::string 						getUri() const;
	std::string 						getHttpVersion() const;
	std::string 						getBody() const;
	std::string 						getHeaders(const std::string& key) const;
	std::string 						getHeader(const std::string& key) const;
	std::string 						getFilename() const;
	std::string 						getFileContent() const;
	const ServerBlock& 					getRequestBlock() const;
	const std::string& 				getContinueResponse() const;
	int 								getStatusCode() const;
	int 								getPort() const;
	// SETTERS
	void 								setFilename(const std::string& filename);
	void 								setFileContent(const std::string& content);
	void 								setStatusCode(int statusCode);
	void 								setPort(int port);

	// OTHER
	std::string							parseHttpMethod(const std::string& methodStr);
	std::map<std::string, std::string>	parseHeaders(std::istringstream& requestStream);
	void 								headersGood();


};