#include "../include/Client.hpp"

Client::Client() : clientSock(-2)
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


void   Client::queueResponse(string response)
{
	this->responses.push_back(response);
}




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

void Client::queueRequest(const std::string& request) {
    requests.push_back(request);
    is_sending = true;
}

const ServerBlock *Client::getServerBlock()
{
    return (this->_block);
}