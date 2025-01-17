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
					msg_receive(this->clients[event]);
				}
				else
				{
					Client& client = *reinterpret_cast<Client*>(eventList[i].udata);
					sendCGIOutput(client);
				}
			}
			else if (eventList[i].filter == EVFILT_USER)
			{
				if (event % 10 == 1)
				{
					debug("Process Event");
					processRequest(this->clients[event/10]);
				}
				else if (event % 10 == 2)
				{
					debug("Send Event");
					msg_send(this->clients[event/10], 0);
				}
				this->removeEvent(event);
			}
			else if (eventList[i].filter == EVFILT_WRITE)
			{
				debug("Write Event");
				msg_send(this->clients[event], 1);  
			}
		}
	}
}

bool isCGIRequest(const HttpRequest &request) {
	// Define CGI-related paths
	const std::vector<std::string> cgiPaths = {"/cgi/", "/cgi-bin/"};

	// Get the URI from the request
	const std::string &uri = request.getUri();

	// Check if the URI matches any CGI path
	for (const std::string &cgiPath : cgiPaths) {
		if (uri.find(cgiPath) == 0) { // Starts with the CGI path
			return true;
		}
	}

	return false;
}

std::string resolveCGIPath(const std::string &uri) {
	const std::string cgiRoot = "www/cgi/";
	const std::string cgiBinRoot = "www/cgi-bin/";

	size_t queryPos = uri.find('?');
	std::string cleanUri = (queryPos != std::string::npos) ? uri.substr(0, queryPos) : uri;

	if (cleanUri.find("/cgi/") == 0) {
		return cgiRoot + cleanUri.substr(5);  // Remove "/cgi/" prefix
	} else if (cleanUri.find("/cgi-bin/") == 0) {
		return cgiBinRoot + cleanUri.substr(9);  // Remove "/cgi-bin/" prefix
	} else {
		throw std::runtime_error("Invalid CGI path: " + uri);
	}
}


void	Server::processRequest(Client &client)
{
	if (!client.hasRequest())
		return;
	string&	req = client.getRequest();
	if (req.find("HTTP") != std::string::npos)
	{
		HttpRequest		request(client);
		if (isCGIRequest(request))
		{
			executeCGI(client, resolveCGIPath(request.getUri()), req);
			return;
		}
		else if (request.getMethod() == "POST")
		{
			string filename = request.getHeader("filename");
			if (filename.empty())
				request.setStatusCode(500); // Check
			client.get_outFile().open("./www/uploads/" + filename, std::ios::binary);//temp
			if (!client.get_outFile().is_open())
				request.setStatusCode(500); // Check
			string contentType = request.getHeader("Content-Type");
			std::regex boundaryRegex("boundary=([a-zA-Z0-9'-]+)");
			std::smatch match;
			if (std::regex_search(contentType, match, boundaryRegex) && match.size() > 1)
			{
				client.get_boundary() = match[1].str();
			}
			else
				request.setStatusCode(500); // Check
			client.isSending() = true; // Used?
		}

		HttpResponse	response(request);
		
		if (request.getMethod()=="POST") //temp
		{
			// debug("Sending 100 CONTINUE");
			// client.queueResponse(request.getContinueResponse());
		}
		else
		{
			client.queueResponse(response.returnResponse());
			this->postEvent(client.getSocket(), 2); //TEMP

		}
		// this->postEvent(client.getSocket(), 2);
	}
	else
	{
		int status = client.processFile();
		if (status)
		{
			client.isSending() = false;
			string response;
			client.get_outFile().close();
			if (!client.get_outFile() || status < 0)
			{
				//create upload failed response
				response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nUpload failed.\r\n";
			}
			//create upload complete response
			response = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\nContent-Length: 22\r\n\r\nUpload successful.\r\n";
			client.queueResponse(response);
			this->postEvent(client.getSocket(), 2);
		}
		// else
		// {
		// 	string	response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\n\r\nOK\r\n";
		// 	client.queueResponse(response);
		// 	this->postEvent(client.getSocket(), 2);
		// }
	}
	client.popRequest();
}

void	Server::acceptClient(int server_sock)
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
	kevent(this->kq, &event, 1, NULL, 0, NULL);
	if (client.isReceiving())
	{
		disable_write_listen(client.getSocket());
	}
	close(client.getSocket());
	this->clients.erase(client.getSocket());
}
void Server::executeCGI(Client &client, const std::string &cgiPath, string &request)
{
	int cgiOutput[2];
	if (pipe(cgiOutput) < 0)
	{
		throw std::runtime_error("Failed to create pipes for CGI");
	}

	client.setPid(fork());
	if (client.getPid() < 0)
	{
		throw std::runtime_error("Failed to fork CGI process");
	}

	if (client.getPid() == 0)
	{ // Child process
		close(cgiOutput[0]); // Close unused read end
		dup2(cgiOutput[1], STDOUT_FILENO); // Redirect CGI output

		const char* python = "/usr/bin/php";
		char* const args[] = {const_cast<char*>(python), const_cast<char*>(cgiPath.c_str()), const_cast<char*>(request.c_str()), nullptr};

		if (execve(python, args, nullptr) == -1)
		{
			_exit(1); // Exit child process if execve fails
		}
	}
	else
	{ // Parent process
		close(cgiOutput[1]); // Close unused write end
		client.setCGIOutput(cgiOutput[0]);
		debug("executin child with parameter:\n" + request);

		// Add the output pipe to the kqueue
		struct kevent event;
		EV_SET(&event, cgiOutput[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
		if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
		{
			close(cgiOutput[0]);
			throw std::runtime_error("Failed to add CGI output to kqueue");
		}

		// Store CGI-related file descriptors in the client
	}
}

void Server::sendCGIOutput(Client &client)
{
	int status;
	pid_t ret;

	ret = waitpid(client.getPid(), &status, WNOHANG);
	if (ret == 0)
	{
		return; // CGI process is still running
	}
	else if (ret == client.getPid())
	{
		struct kevent event;
		
		// Remove the event from the kqueue
		EV_SET(&event, client.getCGIOutputFd(), EVFILT_READ, EV_DELETE, 0, 0, nullptr);
		if (kevent(this->kq, &event, 1, nullptr, 0, nullptr) < 0)
		{
			throw std::runtime_error("Failed to remove CGI output from kqueue");
		}
		if (WIFEXITED(status))
		{
			std::string response;
			char buffer[2048];
			ssize_t bytesRead = read(client.getCGIOutputFd(), buffer, sizeof(buffer));
			
			// Check for read error or empty output
			if (bytesRead < 0)
			{
				throw std::runtime_error("Failed to read CGI output");
			}

			// If there was no data, it's a failure
			std::string output;
			if (bytesRead > 0)
			{
				output.append(buffer, bytesRead); // Append the output from the pipe
			}

			// Build the HTTP response
			if (output.empty())
			{
				response = "HTTP/1.1 500 Internal Server Error\r\n";
			}
			else
			{
				response = "HTTP/1.1 200 OK\r\n";
				response += "Content-Length: " + std::to_string(output.size()) + "\r\n";
				response += "\r\n"; // End of headers
				response.append(output); // Append the CGI output
				response.append("\r\n");
			}
			response.append("\0");
			client.queueResponse(response);
			this->postEvent(client.getSocket(), 2);

			// Close the pipes after processing the output
			close(client.getCGIOutputFd());
		}
		else
		{
			throw std::runtime_error("Child Process Failed");
		}
	}
	else
	{
		throw std::runtime_error("Failed Waitpid");
	}
}

