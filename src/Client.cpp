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


void    Client::queueResponse(const std::string& response)
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
			ssize_t bytesRead = recv(clientSock, buffer, sizeof(buffer), 0); // WRONG
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

void	Client::queueRequest(string request)
{
	debug("New Request added to client : ");
	debug(request);
	this->requests.push_back(request);
}

void	Client::appendRequest(string request)
{
	debug("Line appended to previous request");
	debug(request);
	this->requests.back().append(request);
}

int	Client::processFile()
{
	string	*req = &this->requests.front();

	string	boundaryPrefix = "--" + this->_boundary + "\r\n";
	string	boundarySufix = "--" + this->_boundary + "--\r\n";

	string	endBoundary;

	if (req->substr(0, boundaryPrefix.length()) != boundaryPrefix)
		return -1;

	if (req->substr(req->length() - boundaryPrefix.length()) == boundaryPrefix)
		endBoundary = boundaryPrefix;
	else if (req->substr(req->length() - boundarySufix.length()) == boundarySufix)
		endBoundary = boundarySufix;
	else
		return -1;

	string	content = req->substr(boundaryPrefix.length(), req->length() - endBoundary.length());

	content = content.substr(content.find("\r\n\r\n") + 4);

	if (!this->_outFile || !this->_outFile.is_open())
		return -1;
	debug("Writing into file...");
	this->_outFile << content;
	if (!this->_outFile)
		return -1;

	if (endBoundary == boundarySufix)
		return 1;
	
	// if (this->requests.size() > 1)
	// 	this->requests.at(1) = boundaryPrefix + this->requests.at(1);
	// else
	// this->requests.push_back(boundaryPrefix);

	return (0);
}


/*
	if (is HTTP request)
	{
		if (headers are complete)
		{
			if (has content-length)
			{
				if (body is complete)
					return true
				else
					return false
			}
			else
				return true
		}
		else
		{
			return false
		}
	}
	else
	{
		if (boundary at the end)
			return true
		else
			return false
	}
*/
bool	Client::isLastComplete()
{
	if (requests.empty())
	{
		return true; // No requests to check
	}
	string& req = requests.back();
	if (req.find("HTTP") != std::string::npos)
	{
		size_t headerEnd = req.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			return false;
		}
		if (req.find("POST") != std::string::npos)
			return true;
		std::regex contentLengthRegex("Content-Length: (\\d+)", std::regex::icase);
		std::smatch match;
		if (std::regex_search(req, match, contentLengthRegex))
		{
			size_t contentLength = std::stoul(match[1].str());
			size_t bodyStart = headerEnd + 4;
			if (req.size() >= bodyStart + contentLength)
				return true;
			else
				return false;
		}
		else
			return true;
	}
	else
	{
		string boundaryPrefix = "--" + this->_boundary + "\r\n";
    	string boundarySufix = "--" + this->_boundary + "--\r\n";
		if (req.length() < boundaryPrefix.length() + boundarySufix.length())
			return false;
		else if (req.substr(0, boundaryPrefix.length()) == boundaryPrefix &&
				(req.substr(req.length() - boundaryPrefix.length()) == boundaryPrefix ||
				req.substr(req.length() - boundarySufix.length()) == boundarySufix))
		{
			if (req.substr(req.length() - boundaryPrefix.length()) == boundaryPrefix)
				this->queueRequest(boundaryPrefix);
			return true;
		}
		else
			return false;
	}
}

const ServerBlock *Client::getServerBlock()
{
	return (this->_block);
}

std::ofstream	&Client::get_outFile()
{
	return this->_outFile;
}

string	&Client::get_boundary()
{
	return this->_boundary;
}

bool	Client::hasRequest()
{
	return (!this->requests.empty());
}