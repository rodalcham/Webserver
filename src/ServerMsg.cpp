#include "../include/Server.hpp"

#include <errno.h>

/**
 * Adds a write event to the kqueue
 */
void	Server::enable_write_listen(int	clientSock)
{
	struct kevent enable;
	EV_SET(&enable, clientSock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
	if (kevent(this->kq, &enable, 1, nullptr, 0, nullptr) < 0)
	{
		close(clientSock);
		this->clients.erase(clientSock);
		throw std::runtime_error("Failed to listen for write readiness");
	}
}

/**
 * Removes the write event form the kqueue
 */
void	Server::disable_write_listen(int	clientSock)
{
	struct kevent disable;
	EV_SET(&disable, clientSock, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
	if (kevent(this->kq, &disable, 1, nullptr, 0, nullptr) < 0)
	{
		close(clientSock);
		this->clients.erase(clientSock);
		throw std::runtime_error("Failed disable listen for write readiness");
	}
}

/**
 * Posts custom events,
 * 
 * 0 for read
 * 1 for process
 * 2 for write
 */
void	Server::postEvent(int clientSock, int mode)
{
	struct kevent	event;
	int				ident;

	ident = (clientSock * 10) + mode;
	EV_SET(&event, ident, EVFILT_USER, EV_ADD | EV_ENABLE, NOTE_TRIGGER, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0) {
		throw std::runtime_error("Failed to post custom event to kqueue");
	}

}

void	Server::removeEvent(int eventID)
{
	struct kevent	event;

	EV_SET(&event, eventID, EVFILT_USER, EV_DELETE, 0, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0)
		throw std::runtime_error("Failed to remove custom event from kqueue: " + std::string(strerror(errno)));
}


/**
 * Send message, post event 2 to postpone a send until the previous is finished
 * Posts Write listen to keep sending when not finished
 */
void		Server::msg_send(Client &client, int mode)
{
	size_t	bytes;
	if (!mode && client.isReceiving())
	{
		this->postEvent(client.getSocket(), 2);
		return;
	}

	string	*msg =&client.getResponse();
	debug("Sending message to client : " + *msg);
	// debug("Size : " + std::to_string(msg->size()));
	while (!msg->empty())
	{
		bytes = send(client.getSocket(), msg->data(), msg->size(), 0);
		if	(bytes > 0)
		{
			msg->erase(0, bytes);
			if (msg->empty())
			{
				// debug("Message now empty");
				client.popResponse();
				if (mode)
				{
					disable_write_listen(client.getSocket());
					client.isReceiving() = false;
				}
				return;
			}
		}
		else if (errno == EAGAIN || errno == EWOULDBLOCK) //WRONG !!!!!!!!!
		{
			if (!mode)
			{
				client.isReceiving() = true;
				enable_write_listen(client.getSocket());
			}
			break;
		}
		else
		{
			if (mode)
				disable_write_listen(client.getSocket());
			close(client.getSocket());
			this->clients.erase(client.getSocket());
			throw std::runtime_error("Failed to send to client socket " + std::to_string(client.getSocket()) + ": " + strerror(errno));
		}
	}
}


string extractLine(char buffer[], ssize_t bufferSize)
{
    for (ssize_t i = 0; i < bufferSize; i++)
    {
        if (buffer[i] == '\n')
        {
            return std::string(buffer, i + 1);  // Ensures the string ends with the newline
        }
    }
    // If no newline found, return the whole buffer up to the size
    return std::string(buffer, bufferSize);  
}

void Server::msg_receive(Client& client)
{
	char	buffer[40960];
	memset(buffer, 0, sizeof(buffer));
	ssize_t	bytes_read = recv(client.getSocket(), buffer, sizeof(buffer), 0);
	debug("Bytes read: " + std::to_string(bytes_read));
	ssize_t	pos;
	string	temp;
	
	if (bytes_read < 0)
	{
		debug("Error receiving from client " + std::to_string(client.getSocket()));
		removeClient(client);
	}
	else if (bytes_read == 0)
	{
		debug("Client Closed Connection");
		removeClient(client);
	}
	
	pos = 0;
	while (pos < bytes_read)
	{
		temp = extractLine(buffer + pos, bytes_read - pos);
		// debug("Received : " + temp);
		pos += temp.length();
		if (!client.hasRequest())
		{
			client.queueRequest(temp);
		}
		else
		{
			if (client.isLastComplete())
			{
				client.queueRequest(temp);
			}
			else
			{
				client.appendRequest(temp);
			}
		}
		if (client.isLastComplete())
		{
			// debug("Request Added Succesfully from Client " + std::to_string(client.getSocket()));
			this->postEvent(client.getSocket(), 1);
		}
	}
}

	/*
	Ectract a single request

	If isSending
	{
		If file is open
		{
			write to file
			if final boundary
				close file
		}

		Else
			append to request
	
	}

	Else :
	{	
		If post or incomplete
		{
			set isSending to true
			if post
			{
				Create file
				Store Boundary
			}			
		}
	
		Push to client req deque
	}
	Repeat
	*/

/**
 * Receiving message, posts event 0 to continue reading
 * Post event 1 n times to process n requests
 */
// void Server::msg_receive(Client& client, int flag) {
//     (void)flag; // flag unused

//     char buffer[4096];
//     ssize_t bytesRead = recv(client.getSocket(), buffer, sizeof(buffer), 0);
//     if (bytesRead <= 0)
// 	{
//         std::cerr << "[DEBUG] Connection closed or read error\n";
//         client.closeConnection();
//         clients.erase(client.getSocket());
//         return;
//     }

//     // If we have no partial request yet, create one
//     if (!client.hasPartialRequest())
// 	{
//         HttpRequest* newRequest = new HttpRequest(); //No malloc protection
//         client.setPartialRequest(newRequest);
//         client.setHeadersParsed(false); // Initially, headers not parsed
//         std::cerr << "[DEBUG] Initialized new partial HttpRequest\n";
//     }

//     client.appendToPartialRequestBody(std::string(buffer, bytesRead));
//     std::string& partialBody = client.getPartialRequestBody();
//     HttpRequest& request = client.getPartialRequest();

//     // If headers are already parsed, do NOT parse them again.
//     // Just return here, and let handleIncomingData() or parseBody() handle the rest.
//     if (client.headersParsed()) {
//         return; 
//     }

//     // Headers not parsed yet, check if we have full headers
//     size_t headerEnd = partialBody.find("\r\n\r\n");
//     if (headerEnd != std::string::npos) {
//         std::cerr << "[DEBUG] Headers received, processing request\n";

//         // Extract just the headers portion
//         std::string headerData = partialBody.substr(0, headerEnd + 4);

//         try {
//             // Parse headers by creating a temporary HttpRequest
//             HttpRequest tempReq(headerData, client.getServerBlock());
//             request = tempReq; // Overwrite partial request with parsed headers

//             // Remove the parsed headers from partialBody
//             partialBody.erase(0, headerEnd + 4);

//             // Mark headers as parsed, so we won't parse them again
//             client.setHeadersParsed(true);

//             // If Expect: 100-continue is set, handle it in handleIncomingData()
//             // If POST, we will wait for more data or parseBody() in handleIncomingData().
//         } catch (const std::exception& e) {
//             std::cerr << "[DEBUG] Error parsing headers: " << e.what() << "\n";
//             std::string resp = "HTTP/1.1 400 Bad Request\r\nContent-Length:0\r\n\r\n";
// 			client.queueResponse(resp);
// 			this->postEvent(client.getSocket(), 2);
//             client.clearPartialRequest();
//         }
//     }
// }
