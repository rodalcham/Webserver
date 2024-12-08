#pragma once

#include "Webserv.hpp" 
#include "HTTPRequest.hpp"
#include "ServerBlock.hpp"
#include <vector>
#include <map>

class HttpRequest; // Forward declaration

// /**
//  * A Class representing the server, used to create a socket and listen to inncoming connections or requests.
//  * 
//  * @param serverSock An Integer containing the file descriptor for the server socket.
//  * @param kq An integer representing the KQueue.
//  */
// class Server {
// public:
//     // Server();
//     ~Server();
//     Server(ServerBlock& serverBlock);
//     void run();
//     void handleGet(int clientSock, HttpRequest& httpRequest);
//     void handlePost(int clientSock, HttpRequest& httpRequest);
//     void handleDelete(int clientSock, HttpRequest& httpRequest);
//     std::string resolvePath(const std::string& uri);

// private:
//     ServerBlock serverBlock;
//     int serverSock;
//     int kq;

//     void acceptClient();
//     void handleClient(int clientSock);
//     // void sendResponse(int clientSock, const std::string& body, int statusCode, const std::string& contentType = "text/plain");

//     std::string readFile(const std::string& filePath); // Function to read static files
//     std::string getMimeType(const std::string& filePath);
// };


class Server {
public:
    Server(const std::vector<ServerBlock>& blocks);
    ~Server();

    void run();
    void handleGet(int clientSock, HttpRequest& httpRequest);
    void handlePost(int clientSock, HttpRequest& httpRequest);
    void handleDelete(int clientSock, HttpRequest& httpRequest);
    std::string resolvePath(const std::string& uri, const ServerBlock& block);

private:
    std::vector<ServerBlock> serverBlocks;
    int kq;

    // We will have multiple listening sockets
    std::vector<int> listenSockets;
    std::map<int, ServerBlock*> socketToBlockMap;

    void acceptClient(int listeningSock);
    void handleClient(int clientSock);
    ServerBlock& matchServerBlock(const HttpRequest& httpRequest);

    std::string readFile(const std::string& filePath);
    std::string getMimeType(const std::string& filePath);
};
