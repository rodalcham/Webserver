#pragma once

#include "Webserv.hpp"
#include "ServerBlock.hpp"

using std::string;

/**
 * A class containing client info
 * 
 * @param clienSock The socket id associated with the client
 * @param requests A FIFO array of all the requests coming from this client
 * @param responses A FIFO array of all the responses beint sent to this client
 * @param is_sending A variable to block multiple concurring recvs
 * @param is_receiving A variable to block multiple concurring sends
 * 
 */
class Client
{

	private :

	int						clientSock;
	std::deque<std::string>	requests;
	std::deque<std::string>	responses;
	bool					is_sending = false;
	bool					is_receiving = false;
	const ServerBlock		*_block;


	// PREVIOUS 
	bool headers_parsed = false;

	// Partial request data
	HttpRequest* partialRequest = nullptr; // Pointer to partial request object
	std::string partialRequestBody;        // Body for partial requests
	public : 

	Client();
	Client(int clientSock, const ServerBlock *block);

	int			&getSocket();
	std::string	&getRequest();
	std::string	&getResponse();
	void		popResponse();
	void		popRequest();
	bool		&isSending();
	bool		&isReceiving();
	size_t		parseRequest(char *buffer, int bytesRead);
	void		queueResponse(string response);

// PREVIOUS 
	// Partial Request Management
	bool 			hasPartialRequest() const;
	HttpRequest&	getPartialRequest();
	std::string&	getPartialRequestBody();
	void 			appendToPartialRequestBody(const std::string& bodyChunk);
	void 			setPartialRequest(HttpRequest* request);
	void 			clearPartialRequest();
	void 			handleRequest();
	void 			closeConnection();
	size_t 			parseRequest(char* buffer, int bytesRead);
	void 			queueResponse(const std::string& response);
	void 			queueRequest(const std::string& request);
	bool 			headersParsed() const;
	void 			setHeadersParsed(bool parsed);


};
