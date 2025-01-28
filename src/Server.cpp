#include "../include/Webserv.hpp"
#include "../include/Server.hpp"
#include "../include/Config.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/Client.hpp"

#include <errno.h>
#include <filesystem>
#include <vector>
#include <string>


class log;

extern std::atomic<bool> keepRunning;

uint16_t	ft_htons(uint16_t port);



bool Server::isMethodAllowedInUploads(HttpRequest request, Client &client)
{
	std::string method = request.getMethod();
	const ServerBlock *serverBlock = client.getServerBlock();
	auto locationBlock = serverBlock->getLocationBlock(request.getMatched_location());

	if (request.getMatched_location() == "/return")
		return true;
	if (locationBlock.find("allow_methods") != locationBlock.end())
	{

		std::istringstream iss(locationBlock.at("allow_methods"));
		std::string allowedMethod;
		while (iss >> allowedMethod)
		{
			if (allowedMethod == method)
			{

				return true;
			}
		}
	}


	return false;
}



std::string listUploadsJSON(const std::string &dirPath, string endpoint)
{
	namespace fs = std::filesystem;
	std::vector<std::string> files;

	for (const auto &entry : fs::directory_iterator(dirPath + endpoint))
	{
		if (entry.is_regular_file())
		{
			files.push_back(entry.path().filename().string());
		}
	}
	std::string json = "[";
	for (size_t i = 0; i < files.size(); ++i)
	{
		json += "\"" + files[i] + "\"";
		if (i + 1 < files.size())
			json += ",";
	}
	json += "]";
	return json;
}


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
		this->_server_sockets[sock] = &block;

		int	flags = fcntl(sock, F_GETFL,0);
		if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
			throw std::runtime_error("Failed to set non-blocking mode");

		int	opt = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw std::runtime_error("Failed to set socket options");

		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = ft_htons(block.getPort());
		serverAddr.sin_addr.s_addr = INADDR_ANY;

		if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
			throw std::runtime_error("Failed to bind socket to port " + std::to_string(block.getPort()));

		if (listen(sock, SOCKET_BACKLOG_MAX) < 0)
			throw std::runtime_error("Failed to listen on port "+ std::to_string(block.getPort()));

		struct kevent event;
		EV_SET(&event, sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
		if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
			throw std::runtime_error("Failed to add server socket to kqueue");

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
void	Server::run()
{
	while (keepRunning)
	{
		struct kevent eventList[1024];
		int eventCount = kevent(kq, nullptr, 0, eventList, 1024, nullptr);

		if (eventCount < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("kevent() failed");
		}

		for (int i = 0; i < eventCount; ++i) 
		{
			int event = eventList[i].ident;
			if (eventList[i].filter == EVFILT_READ)
			{
				debug("Read Event");
				if (this->_server_sockets.find(event) != this->_server_sockets.end())
					acceptClient(event);
				else if(this->clients.find(event) != this->clients.end())
				{
					debug("From client");
					msg_receive(this->clients[event]);
				}
				else
				{
					debug("From cgi");
					Client& client = *reinterpret_cast<Client*>(eventList[i].udata);
					sendCGIOutput(client);
				}
			}
			else if (eventList[i].filter == EVFILT_USER)
			{
				if (event % 10 == 2)
				{
					debug("Send Event");
					if (this->clients[event/10].hasResponse())
						msg_send(this->clients[event/10], 0);
					else
						this->removeEvent(event);
				}
				else
				{
					debug("File event");
					if (this->clients[event/10].hasFileContent())
					{
						if (this->clients[event/10].processFile(event % 10))
							this->postEvent(event/10, 2);
						else
							setTimeout(this->clients[event/10]);
					}
					else
						this->removeEvent(event);
				}
			}
			else if (eventList[i].filter == EVFILT_WRITE)
			{
				debug("Write Event");
				msg_send(this->clients[event], 1);  
			}
			else if (eventList[i].filter == EVFILT_TIMER)
			{
				debug("Timeout event");
				removeClient(this->clients[event]);
			}
		}
	}
}

bool isCGIRequest(const HttpRequest &request)
{
	if (request.getHeader("X-Request-Type") == "cgi")
	{
		return true;
	}
	return false;
}

std::string	resolveCGIPath(const std::string &uri)
{
	const std::string cgiRoot = "www/cgi/";
	const std::string cgiBinRoot = "www/cgi-bin/";

	size_t queryPos = uri.find('?');
	std::string cleanUri = (queryPos != std::string::npos) ? uri.substr(0, queryPos) : uri;

	if (cleanUri.find("/cgi/") == 0)
	{
		return cgiRoot + cleanUri.substr(5);
	}
	else if (cleanUri.find("/cgi-bin/") == 0)
	{
		return cgiBinRoot + cleanUri.substr(9);
	}
	else
	{
		throw std::runtime_error("Invalid CGI path: " + uri);
	}
}


bool isHttpRequest(const std::string &request)
{
	size_t newlinePos = request.find('\n');
	if (newlinePos == std::string::npos)
	{

		return false;
	}
	std::string firstLine = request.substr(0, newlinePos);
	if (!firstLine.empty() && firstLine.back() == '\r')
	{
		firstLine.pop_back();
	}
	std::regex httpRequestRegex(R"(^(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH|TRACE|CONNECT) [^\s]+ HTTP/\d\.\d$)");
	return std::regex_match(firstLine, httpRequestRegex);
}


void	Server::acceptClient(int server_sock)
{
	int clientSock = accept(server_sock, nullptr, nullptr);
	if (clientSock < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) 
			return; // No more connections to accept
		throw std::runtime_error("Failed to accept new client");
	}
	this->clients[clientSock] = Client(clientSock, this->_server_sockets[server_sock]);

	// Set client socket to non-blocking mode
	int flags = fcntl(clientSock, F_GETFL, 0);
	if (flags < 0 || fcntl(clientSock, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		removeClient(this->clients[clientSock]);
		throw std::runtime_error("Failed to set client socket to non-blocking mode");
	}

	// Register the client socket with kqueue
	struct kevent event;
	EV_SET(&event, clientSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
	{
		removeClient(this->clients[clientSock]);
		throw std::runtime_error("Failed to add client socket to kqueue");
	}

	setTimeout(this->clients[clientSock]);

	debug("Client accepted : " + std::to_string(clientSock) + 
		"\nConnected through port : " + std::to_string(this->_server_sockets[server_sock]->getPort())
		+ "\nThrough host : " + this->_server_sockets[server_sock]->getHostName());
}

std::string Server::readFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) throw std::runtime_error("File not found");

	std::ostringstream content;
	content << file.rdbuf(); //WRONG??
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

void Server::removeClient(Client &client)
{
	struct kevent event;

	debug("Removing Client " + std::to_string(client.getSocket()));
	EV_SET(&event, client.getSocket(), EVFILT_READ, EV_DELETE, 0, 0, NULL);
	if (kevent(this->kq, &event, 1, NULL, 0, NULL))
		throw std::runtime_error("Failed disable read event");
	EV_SET(&event, client.getSocket(), EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) == -1)
		throw std::runtime_error("Failed disable timeout event");
	if (client.isSending())
		disable_write_listen(client.getSocket());
	close(client.getSocket());
	this->clients.erase(client.getSocket());
}

void	Server::setTimeout(Client &client)
{
	struct kevent event;

	// Overwrite the old timer event with the same ident (clientSocket)
	EV_SET(&event, client.getSocket(), EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, TIMEOUT, nullptr);

	if (kevent(kq, &event, 1, nullptr, 0, nullptr) == -1)
	{
		removeClient(client);
		throw std::runtime_error("Failed to reset timer event");
	}
}

// void Server::executeCGI(Client &client, const std::string &cgiPath, std::string &request)
// {
// 	int cgiOutput[2];
// 	int cgiInput[2];
// 	if (pipe(cgiOutput) < 0 || pipe(cgiInput) < 0)
// 	{
// 		throw std::runtime_error("Failed to create pipes for CGI");
// 	}

// 	client.setPid(fork());
// 	if (client.getPid() < 0)
// 	{
// 		throw std::runtime_error("Failed to fork CGI process");
// 	}

// 	if (client.getPid() == 0)
// 	{
// 		close(cgiOutput[0]);
// 		dup2(cgiOutput[1], STDOUT_FILENO);
// 		close(cgiOutput[1]);
// 		close(cgiInput[1]);

// 		std::string::size_type dotPos = cgiPath.find_last_of('.');
// 		std::string extension;
// 		if (dotPos != std::string::npos)
// 			extension = cgiPath.substr(dotPos + 1); // "py", "php", etc.

// 		const char* interpreter = "/usr/bin/python3";
// 		if (extension == "php")
// 			interpreter = "/usr/bin/php";
// 		else if (extension == "py")
// 			interpreter = "/usr/bin/python3";

// 		char* const args[] = {
// 			const_cast<char*>(interpreter),
// 			const_cast<char*>(cgiPath.c_str()),
// 			const_cast<char*>(request.c_str()),  // optional if your script reads from argv
// 			nullptr
// 		};

// 		if (execve(interpreter, args, nullptr) == -1)
// 		{
// 			_exit(1);
// 		}
// 	}
// 	else
// 	{
// 		// ---------------- PARENT PROCESS ----------------

// 		// We don’t write to cgiOutput[1], so close it
// 		close(cgiOutput[1]);

// 		// If you aren’t writing the raw request to STDIN, close the input pipe
// 		close(cgiInput[0]);
// 		close(cgiInput[1]);

// 		// Save the child’s STDOUT fd so we can read it in `sendCGIOutput`
// 		client.setCGIOutput(cgiOutput[0]);
// 		debug("executing child with parameter:\n" + request);

// 		// Add the child’s STDOUT pipe to the kqueue for reading
// 		struct kevent event;
// 		EV_SET(&event, cgiOutput[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
// 		if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
// 		{
// 			close(cgiOutput[0]);
// 			throw std::runtime_error("Failed to add CGI output to kqueue");
// 		}
// 	}
// }

void Server::sendCGIOutput(Client &client)
{
	int status;
	pid_t ret;
	HttpRequest	request = client.getStoredRequest();
	HttpResponse	res;

	ret = waitpid(client.getPid(), &status, WNOHANG);
	if (ret == 0)
	{
		debug("CGI still running");
		return;
	}
	else if (ret == client.getPid())
	{
		struct kevent event;
		
		client.isExecuting() = false;
		EV_SET(&event, client.getCGIOutputFd(), EVFILT_READ, EV_DELETE, 0, 0, nullptr);
		if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
		{
			throw std::runtime_error("Failed to remove CGI output from kqueue");
		}
		if (WIFEXITED(status) > 0)
		{
			std::string response;
			char buffer[2048];
			ssize_t bytesRead = read(client.getCGIOutputFd(), buffer, sizeof(buffer));
			
			if (bytesRead < 0)
			{
				throw std::runtime_error("Failed to read CGI output");
			}
			std::string output;
			if (bytesRead > 0)
			{
				output.append(buffer, bytesRead); // Append the output from the pipe
			}
			if (output.empty())
			{
				res = HttpResponse(500, "Internal Server Error", request);
			}
			else
			{
				res = HttpResponse(200, output, request);
			}
			client.queueResponse(res.returnResponse());
			postEvent(client.getSocket(), 2);
			// client.popRequest();
		}
		else
		{
			throw std::runtime_error("Child Process Failed");
		}
	}
	else
	{
		client.isExecuting() = false;
		throw std::runtime_error("Failed Waitpid");
	}
}
