// #pragma once

// #include "Webserv.hpp" 
// #include "HTTPRequest.hpp"
// #include "ServerBlock.hpp"
// #include <vector>
// #include <map>

// class HttpRequest; // Forward declaration

// class Server {
// public:
//     Server(const std::vector<ServerBlock>& blocks);
//     ~Server();

//     void run();
//     void handleGet(int clientSock, HttpRequest& httpRequest);
//     void handlePost(int clientSock, HttpRequest& httpRequest);
//     void handleDelete(int clientSock, HttpRequest& httpRequest);
//     // std::string resolvePath(const std::string& uri, const ServerBlock& block);
//     std::string resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig);

// private:
//     std::vector<ServerBlock> serverBlocks;
//     int kq;

//     // We will have multiple listening sockets
//     std::vector<int> listenSockets;
//     std::map<int, ServerBlock*> socketToBlockMap;

//     void acceptClient(int listeningSock);
//     void handleClient(int clientSock);
//     int matchServerBlock(const HttpRequest& httpRequest);

//     std::string readFile(const std::string& filePath);
//     std::string getMimeType(const std::string& filePath);
// };

#pragma once

#include "Webserv.hpp" 
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "ServerBlock.hpp"
#include <vector>
#include <map>
#include <set>

class HttpRequest; // Forward declaration

class Server {
public:
    Server(const std::vector<ServerBlock>& blocks);
    ~Server();

    void run();
    void handleGet(int clientSock, HttpRequest& httpRequest);
    void handlePost(int clientSock, HttpRequest& httpRequest);
    void handleDelete(int clientSock, HttpRequest& httpRequest);

private:
    std::vector<ServerBlock> serverBlocks;
    int kq;

    std::vector<int> listenSockets; // Listening sockets
    std::map<int, ServerBlock*> socketToBlockMap; // Map socket to server block

    void acceptClient(int listeningSock);
    void handleClient(int clientSock);
    int matchServerBlock(const HttpRequest& httpRequest);
    std::string resolvePath(const std::string& uri, const ServerBlock& block, const std::map<std::string, std::string>& locationConfig);
    
};
