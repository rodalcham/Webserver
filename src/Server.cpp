#include "../include/Webserv.hpp"
#include "../include/Server.hpp"
#include "../include/Config.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/Client.hpp"

#include <errno.h>

class log;

extern std::atomic<bool> keepRunning;

uint16_t	ft_htons(uint16_t port);

Server::Server(std::vector<ServerBlock>& server_blocks) : _server_blocks(server_blocks)
{
	this->kq = kqueue();
	if (kq < 0)
		throw	std::runtime_error("Failed to create KQUEUE");

	for (const auto	&block : this->_server_blocks)
	{
		int	sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0)
			throw std::runtime_error("Socket creation failed");

		int	flags = fcntl(sock, F_GETFL,0);
		if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
			throw std::runtime_error("Failed to set non-blocking mode");

		int	opt = 1;
		if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw std::runtime_error("Failed to set socket options");

		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = ft_htons(block.getPort());
		serverAddr.sin_addr.s_addr = INADDR_ANY;

		if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
			throw std::runtime_error("Failed to bind socket to port " + std::to_string(block.getPort()));

		if (listen(sock, SOCKET_BACKLOG_MAX) < 0)
			throw std::runtime_error("Failed to listen on port "+ std::to_string(block.getPort()));

		this->_server_sockets[sock] = &block;
	}
}

Server::~Server()
{
	for (const auto& [sock, block] : this->_server_sockets)
	{
		close(sock);
	}
	for (const auto& [fd, client] : this->clients)
	{
		close(fd);
	}
	close(kq);
}

/** 
 * This is the main server loop, using events to hande different situations and redirecting the program to the correct funtions
*/
void Server::run()
{
	while (keepRunning)
	{
		struct kevent eventList[1024];
		int eventCount = kevent(kq, nullptr, 0, eventList, 1024, nullptr);

		if (eventCount < 0) {
			if (errno == EINTR) continue;
			throw std::runtime_error("kevent() failed");
		}

		for (int i = 0; i < eventCount; ++i) 
		{
			int event = eventList[i].ident;
			if (eventList[i].filter == EVFILT_READ)
			{
				if (this->_server_sockets.find(event) != this->_server_sockets.end())
					acceptClient(event);
				else
				{
					msg_receive(this->clients.at(event), 0);
				}
			}
			else if (eventList[i].filter == EVFILT_USER)
			{
				if (event % 10 == 0)
					msg_receive(this->clients[event/10], 1);
				else if (event % 10 == 1)
				{
					HttpRequest		request(this->clients[event/10].getRequest(), this->_server_blocks);
					HttpResponse	response(request);
					this->clients[event/10].popRequest();
					this->clients[event/10].queueResponse(response.returnResponse());
					this->postEvent(event/10, 2);
					response.debug();
				}
				else if (event % 10 == 2)
					msg_send(this->clients[event/10], 0);
				this->removeEvent(event);
			}
			else if (eventList[i].filter == EVFILT_WRITE)
			{
				msg_send(this->clients[event], 1);  
			}
		}
	}
}

void Server::acceptClient(int server_sock)
{
	int clientSock = accept(server_sock, nullptr, nullptr);
	if (clientSock < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) 
			return; // No more connections to accept
		throw std::runtime_error("Failed to accept new client");
	}

	// Set client socket to non-blocking mode
	int flags = fcntl(clientSock, F_GETFL, 0);
	if (flags < 0 || fcntl(clientSock, F_SETFL, flags | O_NONBLOCK) < 0) {
		close(clientSock);
		throw std::runtime_error("Failed to set client socket to non-blocking mode");
	}

	// Register the client socket with kqueue
	struct kevent event;
	EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0) {
		close(clientSock);
		throw std::runtime_error("Failed to add client socket to kqueue");
	}

	this->clients[clientSock] = Client(clientSock, this->_server_sockets[server_sock]);
	debug("Client accepted : " + std::to_string(clientSock) + 
		"\nConnected through port : " + std::to_string(this->_server_sockets[server_sock]->getPort())
		+ "\nThrough host : " + this->_server_sockets[server_sock]->getHostName());
}


std::string Server::readFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) throw std::runtime_error("File not found");

	std::ostringstream content;
	content << file.rdbuf();
	return content.str();
}

std::string Server::getMimeType(const std::string& filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos) return "application/octet-stream";

	std::string extension = filePath.substr(dotPos + 1);

	if (extension == "html" || extension == "htm") return "text/html";
	if (extension == "css") return "text/css";
	if (extension == "js") return "application/javascript";
	if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
	if (extension == "png") return "image/png";
	if (extension == "gif") return "image/gif";
	if (extension == "txt") return "text/plain";

	return "application/octet-stream";
}
