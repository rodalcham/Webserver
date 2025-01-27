#pragma once

#include "Webserv.hpp" 
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Client.hpp"
#include "ServerBlock.hpp"
#include <vector>
#include <map>
#include <set>

class HttpRequest; // Forward declaration
class HttpResponse; // Forward declaration
class Client;

using std::string;

/**
 * A Class representing the server, used to create a socket and listen to inncoming connections or requests.
 * 
 * @param serverSock An Integer containing the file descriptor for the server socket.
 * @param kq An integer representing the KQueue.
 * @param clients A map containing all the clients, indexedy their socket fd.
 * @param serverBlock The config block for this server
 */
class Server {

	public:

	// Server();
	~Server();
	Server(std::vector<ServerBlock>& server_blocks);
	void	run();
	void	processRequest(Client &client);
	// std::string resolvePath(const std::string& uri);

	private:

	std::vector<ServerBlock> 	_server_blocks;
	int			 				serverSock;
	std::map<int, const ServerBlock*>			_server_sockets;
	int 						kq;
	std::map<int, Client>		clients;

	void			acceptClient(int server_sock);
	void			removeClient(Client &client);
	void			setTimeout(Client &client);
	void			executeCGI(Client &client, const std::string &cgiPath, std::string &request);
	void			sendCGIOutput(Client &client);
	bool			isMethodAllowedInUploads(HttpRequest request, Client &client);
	void			handleRedirect(Client &client);
	std::string 	handleDefaultIndex(Client &client, const std::string &uri);
	std::string 	handleAutoIndex(Client &client, const std::string &uri, HttpRequest request);
	// void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

	std::string readFile(const std::string& filePath); // Function to read static files
	std::string getMimeType(const std::string& filePath);
	std::string handleDirectoryOrFile(const std::string &uri, HttpRequest &request);

	void	enable_write_listen(int clientSock);
	void	disable_write_listen(int clientSock);
	void	msg_send(Client &client, int mode);
	void	msg_receive(Client &client);

	//Posting events
	void	postEvent(int clientSock, int mode);
	void	removeEvent(int eventID);

	//Handle Events
	int				handleFileContent(Client &client, string &req);
	HttpResponse	handleGet(HttpRequest &request, Client &client);
	HttpResponse	handleDelete(HttpRequest &request, Client &client);
	int				handlePost(HttpRequest &request, Client &client);
	int				handleCGI(HttpRequest &request, Client *client, string &req);
	HttpResponse	retrieveFile(HttpRequest &request);

};
