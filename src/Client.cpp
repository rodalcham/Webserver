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
	this->requests.push_back(request);
}

void	Client::appendRequest(string request)
{
	this->requests.back().append(request);
}

int Client::processFile()
{
    string *req = &this->requests.front();

    // debug("CHUNK : ");
    // debug(*req);

    string boundaryPrefix = "--" + this->_boundary + "\r\n";
    string boundarySuffix = "--" + this->_boundary + "--\r\n";
    string *endBoundary;

    // Ensure that the chunk begins with the correct boundary prefix
    if (req->substr(0, boundaryPrefix.length()) != boundaryPrefix)
        return -1;

    // Check if the chunk ends with a boundary prefix or suffix
    if (req->substr(req->length() - boundaryPrefix.length()) == boundaryPrefix)
        endBoundary = &boundaryPrefix;
    else if (req->substr(req->length() - boundarySuffix.length()) == boundarySuffix)
	{
		debug("LAST CHUNK RECEIVED");
        endBoundary = &boundarySuffix;
	}
    else
        return -1;

    // Extract the content of the chunk, excluding the boundary markers
    string content = req->substr(boundaryPrefix.length(), req->length());
	content = content.substr(0, content.length() - endBoundary->length());
	content = content.substr(0, content.length() - 2);

    // Remove the multipart headers (if they exist)
    size_t headerEndPos = content.find("\r\n\r\n");
    if (headerEndPos != string::npos) {
        content = content.substr(headerEndPos + 4); // Skip past the headers
    }

    // Open the file for writing if it's not already open
    if (!this->_outFile || !this->_outFile.is_open())
        return -1;

    debug("Writing into file...");

    // Write the extracted content (this is the file data) into the file
	// debug(content);
    this->_outFile << content;

    // Check if the file was successfully written
    if (!this->_outFile)
        return -1;

    // If the chunk ends with the final boundary, return 1 to signal completion
    if (*endBoundary == boundarySuffix)
        return 1;

    return 0;
}


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
			else
				debug("LAST CHUNK RECEIVED");
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

void Client::setCGIPipes(int inputFd, int outputFd) {
        cgiInputFd = inputFd;
        cgiOutputFd = outputFd;
    }

int Client::getCGIInputFd() const { return cgiInputFd; }
int Client::getCGIOutputFd() const { return cgiOutputFd; }