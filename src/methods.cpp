// #include "../include/Server.hpp"
// #include "../include/HTTPRequest.hpp"
// #include "../include/HTTPResponse.hpp"
// #include "../include/cgi.hpp"
// #include <iostream>
// #include <fstream>
// #include <limits.h> // For PATH_MAX

// // GET Handler
// void Server::handleGet(int clientSock, HttpRequest& httpRequest) {
//     int i = matchServerBlock(httpRequest);
//     if (i < 0) {
//         std::cerr << "[ERROR] No matching server block found.\n";
//         close(clientSock);
//         return;
//     }

//     const auto& locations = serverBlocks[i].location_blocks;
//     std::string matchedLocation = "/";
//     size_t longestMatch = 0;

//     for (const auto& location : locations) {
//         if (httpRequest.getUri().rfind(location.first, 0) == 0 && location.first.size() > longestMatch) {
//             matchedLocation = location.first;
//             longestMatch = location.first.size();
//         }
//     }

//     const auto& locationConfig = locations.at(matchedLocation);
//     std::string filePath = resolvePath(httpRequest.getUri(), serverBlocks[i], locationConfig);
//     httpRequest.setFilePath(filePath);

//     char realPath[PATH_MAX];
//     if (realpath(filePath.c_str(), realPath) == nullptr) {
//         // Invalid path or file not found
//         HttpResponse response(httpRequest, 404, "Not Found");
//         response.sendResponse(clientSock);
//         return;
//     }

//     filePath = std::string(realPath);
//     httpRequest.setFilePath(filePath);

//     // If filePath == rootDir exactly, append /index.html
//     if (filePath == httpRequest.getRootDir()) {
//         filePath += "/index.html";
//         httpRequest.setFilePath(filePath);
//     }

//     if (!std::ifstream(filePath).good()) {
//         HttpResponse response(httpRequest, 404, "Not Found");
//         response.sendResponse(clientSock);
//         return;
//     }

//     HttpResponse response(httpRequest);
//     response.sendResponse(clientSock);
// }

// // DELETE Handler
// void Server::handleDelete(int clientSock, HttpRequest& httpRequest) {
//     try {
//         int i = matchServerBlock(httpRequest);
//         if (i < 0) {
//             std::cerr << "[ERROR] No matching server block found.\n";
//             close(clientSock);
//             return;
//         }

//         const auto& locations = serverBlocks[i].location_blocks;
//         std::string matchedLocation = "/";
//         size_t longestMatch = 0;

//         for (const auto& location : locations) {
//             if (httpRequest.getUri().rfind(location.first, 0) == 0 && location.first.size() > longestMatch) {
//                 matchedLocation = location.first;
//                 longestMatch = location.first.size();
//             }
//         }

//         const auto& locationConfig = locations.at(matchedLocation);
//         std::string filePath = resolvePath(httpRequest.getUri(), serverBlocks[i], locationConfig);

//         std::ifstream file(filePath);
//         if (!file.good()) {
//             // 404 Not Found
//             HttpResponse response(httpRequest, 404, "Not Found");
//             response.sendResponse(clientSock);
//         } else {
//             file.close();
//             if (remove(filePath.c_str()) != 0) {
//                 // Failed to delete -> 500
//                 HttpResponse response(httpRequest, 500, "Internal Server Error");
//                 response.sendResponse(clientSock);
//             } else {
//                 // Successfully deleted -> 200
//                 HttpResponse response(httpRequest, 200, "OK");
//                 response.sendResponse(clientSock);
//             }
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Error handling DELETE request: " << e.what() << std::endl;
//         HttpResponse response(httpRequest, 500, "Internal Server Error");
//         response.sendResponse(clientSock);
//     }
// }

// // POST Handler
// void Server::handlePost(int clientSock, HttpRequest& httpRequest) {
//     auto expectHeader = httpRequest.getHeader("expect");
//     if (!expectHeader.empty()) {
//         std::string expectValue = expectHeader;
//         std::transform(expectValue.begin(), expectValue.end(), expectValue.begin(), ::tolower);
//         if (expectValue == "100-continue") {
//             std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
//             write(clientSock, continueResponse.c_str(), continueResponse.size());
//         }
//     }

//     HttpResponse response(httpRequest, 200, "OK");
//     response.sendResponse(clientSock);
// }
