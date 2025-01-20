#pragma once

#include "Webserv.hpp"
#include "ServerBlock.hpp"
#include <deque>
#include <regex>
#include <fstream>


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

	string					_boundary;
	std::ofstream			_outFile;
	pid_t				pid;
    int cgiOutputFd;  // For reading from CGI process


	public : 

	Client();
	Client(int clientSock, const ServerBlock *block);

	int					&getSocket();
	std::string			&getRequest();
	bool				hasRequest();
	std::string			&getResponse();
	const ServerBlock	*getServerBlock();
	void				popResponse();
	void				popRequest();
	bool				&isSending();
	bool				&isReceiving();

	void setCGIOutput(int outputFd);

    int getCGIOutputFd() const;
	pid_t getPid() const;
	void setPid(pid_t newPid);


// PREVIOUS 
	// Partial Request Management







	void 			closeConnection();
	void 			queueResponse(const std::string& response);
	void 			queueRequest(string request);
	void			appendRequest(string request);

	bool			isLastComplete();


	int				processFile();
	std::ofstream	&get_outFile();
	string			&get_boundary();


};
