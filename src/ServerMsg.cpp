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
		debug("Failed to listen for write readiness");
		removeClient(this->clients[clientSock]);
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
		debug("Failed disable listen for write readiness");
		removeClient(this->clients[clientSock]);
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
		debug("Failed to post custom event to kqueue");
		removeClient(this->clients[clientSock]);
	}

}

void	Server::removeEvent(int eventID)
{
	struct kevent	event;

	EV_SET(&event, eventID, EVFILT_USER, EV_DELETE, 0, 0, nullptr);
	if (kevent(kq, &event, 1, nullptr, 0, nullptr) < 0) {
		debug("Failed to remove custom event from kqueue: " + std::string(strerror(errno)));
		removeClient(this->clients[eventID/10]);
	}
}


/**
 * Send message, post event 2 to postpone a send until the previous is finished
 * Posts Write listen to keep sending when not finished
 */
void		Server::msg_send(Client &client, int mode)
{
	ssize_t	bytes;
	if (!mode && client.isReceiving())
	{
		this->postEvent(client.getSocket(), 2);
		return;
	}

	string	*msg =&client.getResponse();
	debug("Sending message to client : " + *msg);
	while (!msg->empty())
	{
		bytes = send(client.getSocket(), msg->data(), msg->size(), 0);
		if	(bytes > 0)
		{
			setTimeout(client);
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
				if (mode == 2)
					removeClient(client);
				return;
			}
		}
		else
		{
			if (!mode)
			{
				client.isReceiving() = true;
				enable_write_listen(client.getSocket());
			}
			break;
		}
	}
}


string extractLine(char buffer[], ssize_t bufferSize)
{
    for (ssize_t i = 0; i < bufferSize; i++)
    {
        if (buffer[i] == '\n')
        {
            return std::string(buffer, i + 1);
        }
    }
    return std::string(buffer, bufferSize);  
}

void Server::msg_receive(Client& client)
{
	char	buffer[40960];
	memset(buffer, 0, sizeof(buffer));
	ssize_t	bytes_read = recv(client.getSocket(), buffer, sizeof(buffer), 0);
	ssize_t	pos;
	string	temp;
	
	if (bytes_read < 0)
	{
		debug("Error receiving from client " + std::to_string(client.getSocket()));
		removeClient(client);
		return;
	}
	else if (bytes_read == 0)
	{
		debug("Client Closed Connection");
		removeClient(client);
		return;
	}
	setTimeout(client);
	
	pos = 0;
	while (pos < bytes_read)
	{
		temp = extractLine(buffer + pos, bytes_read - pos);
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
			this->processRequest(client);
		}
	}
}
