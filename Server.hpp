#ifndef SERVER_HPP
#define SERVER_HPP

#include "Webserv.hpp" 
#include "HTTPRequest.hpp"


class Server {
public:
    Server();
    ~Server();
    void run();
    void handleGet(int clientSock, HttpRequest& httpRequest);
    void handlePost(int clientSock, HttpRequest& httpRequest);
    void handleDelete(int clientSock, HttpRequest& httpRequest);
    std::string resolvePath(const std::string& uri);

private:
    int serverSock;
    int kq;

    void acceptClient();
    void handleClient(int clientSock);
    void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

    std::string readFile(const std::string& filePath); // Function to read static files
    std::string getMimeType(const std::string& filePath);
};

#endif // SERVER_HPP
