#include "../include/Client.hpp"

bool isHttpRequest(const std::string &request);

Client::Client() : clientSock(-3)
{

}

Client::Client(int clientSock, const ServerBlock *block)
	: clientSock(clientSock),_block(block)
{

}

bool	Client::isIdle()
{
	if (!is_executing && !is_receiving && ! is_sending && requests.empty() &&
		responses.empty() && _file_content.empty())
		return true;
	return false;
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

bool	&Client::hasTimeout()
{
	return (this->_timeout);
}

bool	&Client::isExecuting()
{
	return (this->is_executing);
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
    if (!this->is_sending)
    {
        this->_file_content.pop_front();
        return (0);
    }

	string *req = &this->_file_content.front();

	if (!this->_outFile || !this->_outFile.is_open())
    {
		HttpResponse	failure(201, "Upload failed", this->_stored_request);
        this->_outFile.close();
        this->queueResponse(failure.returnResponse());
        this->_file_content.pop_front();
        this->is_sending = false;
		return -1;
    }

	debug("Writing into file...");
	this->_outFile << *req;
    this->_file_content.pop_front();

    if (mode == 4)
    {
        this->is_sending = false;
        this->_outFile.close();
        if(!this->_outFile)
        {
			HttpResponse	failure(201, "Upload failed", this->_stored_request);
            this->queueResponse(failure.returnResponse());
            return -1;
        }
		HttpResponse	success(201, "Upload successful", this->_stored_request);
        this->queueResponse(success.returnResponse());
        return 1;
    }

	if (!this->_outFile)
    {
		HttpResponse	failure(201, "Upload failed", this->_stored_request);
        this->is_sending = false;
        this->queueResponse(failure.returnResponse());
        this->_outFile.close();
		return -1;
    }

	return 0;
}

std::string extractBoundary(const std::string& postRequest)
{
	const std::string contentTypeHeader = "Content-Type: ";
	const std::string boundaryKey = "boundary=";


	size_t contentTypePos = postRequest.find(contentTypeHeader);
	if (contentTypePos == std::string::npos)
	{
		return "";
	}

	// Find the boundary key within the Content-Type header
	size_t boundaryPos = postRequest.find(boundaryKey, contentTypePos);
	if (boundaryPos == std::string::npos)
	{
		return "";
	}

	size_t boundaryStart = boundaryPos + boundaryKey.length();
	size_t boundaryEnd = postRequest.find("\r\n", boundaryStart);
	if (boundaryEnd == std::string::npos)
	{
		boundaryEnd = postRequest.length();
	}

	return postRequest.substr(boundaryStart, boundaryEnd - boundaryStart);
}

bool	Client::isLastComplete()
{
	if (requests.empty())
	{
		return true;
	}
	string& req = requests.back();
	if (isHttpRequest(req))
	{
		size_t headerEnd = req.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			return false;
		}
		if (req.find("POST") != std::string::npos && req.find("X-request-type") == string::npos)
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

HttpRequest	&Client::getStoredRequest()
{
	return this->_stored_request;
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
