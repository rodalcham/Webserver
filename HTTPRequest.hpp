#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "Webserv.hpp" 


// Enum for supported HTTP methods
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    UNKNOWN // Add UNKNOWN for unsupported or invalid methods
};

// Class to represent an HTTP Request
class HttpRequest {
public:
    HttpRequest();

    ~HttpRequest();

    HttpMethod method;                           // HTTP method (e.g., GET, POST)
    std::string uri;                             // Requested URI
    std::string httpVersion;                     // HTTP version (e.g., HTTP/1.1)
    std::map<std::string, std::string> headers;  // Headers as key-value pairs
    std::string body;                            // Request body (e.g., for POST)

    void debugPrint() const;
    // Helper function to get a header value in a case-insensitive manner
    std::string getHeader(const std::string& headerName) const {
        // Normalize header name to lowercase for case-insensitive comparison
        std::string normalizedHeader = headerName;
        std::transform(normalizedHeader.begin(), normalizedHeader.end(), normalizedHeader.begin(), ::tolower);

        auto it = headers.find(normalizedHeader);
        if (it != headers.end()) {
            return it->second; // Return the header value if found
        }
        return ""; // Return empty string if header not found
    }
private:
    // Helper function to convert HttpMethod to string
    std::string methodToString(HttpMethod method) const;
};

// Function to parse an HTTP request string into an HttpRequest object
HttpRequest parseHttpRequest(const std::string &request);

    std::string parseName(const std::string &body);
    std::string parseContent(const std::string &body);
#endif // HTTPREQUEST_HPP
