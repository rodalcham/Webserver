#include "../include/Client.hpp"

Client::Client() : clientSock(-2)
{

}

Client::Client(int clientSock, const ServerBlock *block) : clientSock(clientSock) _block(block, )
{

}
Client::Client(int clientSock, const ServerBlock *block)
    : clientSock(clientSock),_block(block),partialRequest(nullptr)
{

}


int		&Client::getSocket()
{
	return (this->clientSock);
}

string	&Client::getRequest()
{
	return (this->requests.front());
}

string	&Client::getResponse()
{
	return (this->responses.front());
}

void	Client::popResponse()
{
	this->responses.pop_front();
}

void	Client::popRequest()
{
	this->requests.pop_front();
}

bool	&Client::isSending()
{
	return (this->is_sending);
}

bool	&Client::isReceiving()
{
	return (this->is_receiving);
}

size_t	Client::parseRequest(char *buffer, int bytesRead)
{
	size_t	count = 0;
	static const string delimiter = "\r\n\r\n"; // HTTP request delimiter
	int i = 0;
	string	request;
	while (i < bytesRead)
	{
		string	temp(buffer + i, bytesRead - i); // wasteful, maybe declare outside?
		size_t	end = temp.find(delimiter);
		if (end != string::npos)
			request = temp.substr(0, end + delimiter.size());
		else
		{
			request = temp.substr(0, bytesRead - i);
			i = bytesRead;
		}
		if (this->is_sending)
			this->requests.back() += request;
		else
			this->requests.push_back(request);
		if (end != string::npos)
		{
			this->is_sending = false;
			i += end + delimiter.size();
			count++;
			//post event, maybe return a counter to handle posting in server::recv
		}
		else
			this->is_sending = true;
	}
	return (count);
}
	//Extract a part of the buffer until a /r/n/r/n or the end into a string

	//if isSending is true, add that part into the last request in the queue
	//else make a new request at the back and add it there

	//if the request is not complete, set isSending to true
	//else create an event to proccess the completed request

	//repeat until the buffer is empty

void   Client::queueResponse(string response)
{
	this->responses.push_back(response);
}





// Client::Client() : clientSock(-1), partialRequest(nullptr) {}

// Client::Client(int clientSock) : clientSock(clientSock), partialRequest(nullptr) {}


// Partial Request Handling

bool Client::hasPartialRequest() const
{
    return partialRequest != nullptr;
}

HttpRequest& Client::getPartialRequest()
{
    if (!partialRequest) {
        throw std::runtime_error("No partial request available");
    }
    return *partialRequest;
}

std::string& Client::getPartialRequestBody()
{
    return partialRequestBody;
}

void Client::appendToPartialRequestBody(const std::string& bodyChunk)
{
    partialRequestBody += bodyChunk;
}

void Client::setPartialRequest(HttpRequest* request)
{
    clearPartialRequest(); // Clean up any existing partial request
    partialRequest = request;
}

void Client::clearPartialRequest()
{
    if (partialRequest) {
        delete partialRequest; // Free memory
        partialRequest = nullptr;
    }
    partialRequestBody.clear();
}

void Client::closeConnection()
{
    close(clientSock); // Close the socket
    clientSock = -1;   // Mark as closed
}

void Client::handleRequest()
{
    if (!hasPartialRequest())
	{
        std::cerr << "[DEBUG] No partial request to handle\n";
        return;
    }

    try
	{
        HttpRequest& request = getPartialRequest();
        size_t expectedLength = std::stoul(request.getHeaders("content-length"));

        while (partialRequestBody.size() < expectedLength)
		{
            char buffer[4096];
            ssize_t bytesRead = recv(clientSock, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0)
			{
                std::cerr << "[DEBUG] Connection closed or read error\n";
                closeConnection();
                return;
            }
            partialRequestBody.append(buffer, bytesRead);
            std::cerr << "[DEBUG] Accumulated Body: " << partialRequestBody.size()
                      << " / Expected: " << expectedLength << "\n";
        }
        if (partialRequestBody.size() == expectedLength)
		{
            std::cerr << "[DEBUG] Request fully received\n";
            clearPartialRequest(); // Finalize the request and clean up
        }
    } catch (const std::exception& e)
	{
        std::cerr << "[ERROR] Failed to handle request: " << e.what() << "\n";
        closeConnection();
    }
}

size_t Client::parseRequest(char* buffer, int bytesRead)
{
    std::string incoming(buffer, bytesRead);
    size_t processedRequests = 0;

    if (this->hasPartialRequest())
	{
        this->appendToPartialRequestBody(incoming);
        return 0; // Body handling logic should handle this
    }

    // Example logic to parse complete headers
    size_t headerEnd = incoming.find("\r\n\r\n");
    if (headerEnd != std::string::npos)
	{
        std::string fullRequest = incoming.substr(0, headerEnd + 4);
        this->queueRequest(fullRequest);
        processedRequests++;
    }

    return processedRequests;
}

    bool Client::headersParsed() const
    {
        return headers_parsed;
    }
    void Client::setHeadersParsed(bool parsed)
    {
        headers_parsed = parsed;
    }

	void Client::queueResponse(const std::string& response) {
    responses.push_back(response);
}

void Client::queueRequest(const std::string& request) {
    requests.push_back(request);
    is_sending = true;
}