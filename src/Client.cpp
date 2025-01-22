#include "../include/Client.hpp"

Client::Client() : clientSock(-3)
{

}

Client::Client(int clientSock, const ServerBlock *block)
	: clientSock(clientSock),_block(block)
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

void Client::closeConnection()
{
	close(clientSock); // Close the socket
	clientSock = -1;   // Mark as closed
}

void	Client::queueFileContent(const string &request)
{
	this->_file_content.push_back(request);
}

void	Client::queueRequest(string request)
{
	this->requests.push_back(request);
}

void	Client::appendRequest(string request)
{
	this->requests.back().append(request);
}

int Client::processFile(int mode)
{
	string *req = &this->_file_content.front();
    const string  success = "HTTP/1.1 201 Created\r\nContent-Type: text/plain\r\nContent-Length: 22\r\n\r\nUpload successful.\r\n";
    const string  failure = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 18\r\n\r\nUpload failed.\r\n";

	// debug("CHUNK : ");
	// debug(*req);

	// string boundaryPrefix = "--" + this->_boundary + "\r\n";
	// string boundarySuffix = "--" + this->_boundary + "--\r\n";
	// string *endBoundary;

	// // Ensure that the chunk begins with the correct boundary prefix
	// if (req->substr(0, boundaryPrefix.length()) != boundaryPrefix)
	// 	return -1;

	// // Check if the chunk ends with a boundary prefix or suffix
	// if (req->substr(req->length() - boundaryPrefix.length()) == boundaryPrefix)
	// 	endBoundary = &boundaryPrefix;
	// else if (req->substr(req->length() - boundarySuffix.length()) == boundarySuffix)
	// {
	// 	debug("LAST CHUNK RECEIVED");
	// 	endBoundary = &boundarySuffix;
	// }
	// else
	// 	return -1;

	// // Extract the content of the chunk, excluding the boundary markers
	// string content = req->substr(boundaryPrefix.length(), req->length());
	// content = content.substr(0, content.length() - endBoundary->length());
	// content = content.substr(0, content.length() - 2);

	// // Remove the multipart headers (if they exist)
	// size_t headerEndPos = content.find("\r\n\r\n");
	// if (headerEndPos != string::npos) {
	// 	content = content.substr(headerEndPos + 4); // Skip past the headers
	// }

	// Open the file for writing if it's not already open
	if (!this->_outFile || !this->_outFile.is_open())
    {
        this->_outFile.close();
        this->queueResponse(failure);
		return -1;
    }

	debug("Writing into file...");

	// Write the extracted content (this is the file data) into the file
	// debug(content);
	this->_outFile << *req;
    this->_file_content.pop_front();
    if (mode == 4)
    {
        this->_outFile.close();
        if(!this->_outFile)
        {
            this->queueResponse(failure);
            return -1;
        }
        this->queueResponse(success);
        return 1;
    }

	// Check if the file was successfully written
	if (!this->_outFile)
    {
        this->queueResponse(failure);
        this->_outFile.close();
		return -1;
    }
	// If the chunk ends with the final boundary, return 1 to signal completion
	// if (*endBoundary == boundarySuffix)
	// 	return 1;

	return 0;
}

std::string extractBoundary(const std::string& postRequest)
{
	const std::string contentTypeHeader = "Content-Type: ";
	const std::string boundaryKey = "boundary=";

	// Find the Content-Type header
	size_t contentTypePos = postRequest.find(contentTypeHeader);
	if (contentTypePos == std::string::npos)
	{
		return ""; // Content-Type header not found
	}

	// Find the boundary key within the Content-Type header
	size_t boundaryPos = postRequest.find(boundaryKey, contentTypePos);
	if (boundaryPos == std::string::npos)
	{
		return ""; // Boundary key not found
	}

	// Extract the boundary value
	size_t boundaryStart = boundaryPos + boundaryKey.length();
	size_t boundaryEnd = postRequest.find("\r\n", boundaryStart);
	if (boundaryEnd == std::string::npos)
	{
		boundaryEnd = postRequest.length(); // End of string if no newline
	}

	return postRequest.substr(boundaryStart, boundaryEnd - boundaryStart);
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
		if (req.find("POST") != std::string::npos && req.find("cgi") == string::npos)
		{
			this->_boundary = extractBoundary(req);
			return true;
		}
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

bool	Client::hasResponse()
{
	return (!this->responses.empty());
}

bool	Client::hasFileContent()
{
	return (!this->_file_content.empty());
}

void Client::setCGIOutput(int outputFd)
{
	this->cgiOutputFd = outputFd;
}

int Client::getCGIOutputFd() const
{ 
	return cgiOutputFd;
}

// Getter
pid_t Client::getPid() const
{
	return pid;
}

// Setter
void Client::setPid(pid_t newPid)
{
	this->pid = newPid;
}
