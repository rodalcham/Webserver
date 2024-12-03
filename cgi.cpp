// #include "cgi.hpp"
#include "Webserv.hpp"
// #include "Server.hpp"

std::string resolvePath(const std::string& uri) {
    std::string filePath = ROOT_DIR + uri;
    char realPath[PATH_MAX];
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        perror("realpath failed");
        return "";
    }
    return std::string(realPath);
}


// Convert HttpMethod enum to string
std::string methodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}

// Check if the URI corresponds to a CGI request
bool isCGIRequest(const std::string& uri) {
    std::vector<std::string> cgiExtensions = {".php", ".py"};
    for (const auto& ext : cgiExtensions) {
        if (uri.size() >= ext.size() &&
            uri.compare(uri.size() - ext.size(), ext.size(), ext) == 0) {
            return true;
        }
    }
    return false;
}


// Helper function to get header value in a case-insensitive manner
std::string getHeaderValue(const std::map<std::string, std::string>& headers, const std::string& key) {
    for (const auto& [headerKey, headerValue] : headers) {
        if (strcasecmp(headerKey.c_str(), key.c_str()) == 0) {
            return headerValue;
        }
    }
    return "";
}

// Update buildCGIEnvironment to use getHeaderValue
std::map<std::string, std::string> buildCGIEnvironment(const HttpRequest& httpRequest, const std::string& scriptPath) {
    std::map<std::string, std::string> env;
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["REQUEST_METHOD"] = methodToString(httpRequest.method);
    env["SCRIPT_NAME"] = scriptPath;
    env["PATH_INFO"] = scriptPath;

    size_t queryPos = httpRequest.uri.find('?');
    env["QUERY_STRING"] = (queryPos != std::string::npos) ? httpRequest.uri.substr(queryPos + 1) : "";

    env["CONTENT_TYPE"] = getHeaderValue(httpRequest.headers, "Content-Type");
    env["CONTENT_LENGTH"] = getHeaderValue(httpRequest.headers, "Content-Length");

    return env;
}



// Unchunk a request body if it is chunked
std::string unchunkBody(const std::string& body) {
    std::istringstream stream(body);
    std::ostringstream unchunkedBody;
    std::string line;
    while (std::getline(stream, line)) {
        size_t chunkSize = std::stoul(line, nullptr, 16); // Hex size
        if (chunkSize == 0) break; // End of chunks
        char buffer[chunkSize];
        stream.read(buffer, chunkSize);
        unchunkedBody.write(buffer, chunkSize);
        stream.ignore(2); // Skip \r\n
    }
    return unchunkedBody.str();
}

// Capture output from the CGI script
void captureCGIOutput(int pipeFd, std::string& output) {
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(pipeFd, buffer, sizeof(buffer))) > 0) {
        output.append(buffer, bytesRead);
    }
}

// Handle CGI execution for the given HTTP request
void handleCGI(int clientSock, const HttpRequest& httpRequest) {
    std::string scriptPath = resolvePath(httpRequest.uri);
    std::string scriptDir = scriptPath.substr(0, scriptPath.find_last_of('/'));

    int inPipe[2], outPipe[2];
    if (pipe(inPipe) == -1 || pipe(outPipe) == -1) {
        std::cerr << "Pipe creation failed!" << std::endl;
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        close(inPipe[1]);
        close(outPipe[0]);

        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);

        chdir(scriptDir.c_str());

        std::map<std::string, std::string> cgiEnv = buildCGIEnvironment(httpRequest, scriptPath);

        // Corrected environment variable construction
        std::vector<std::string> envStrings;
        std::vector<char*> envp;
        for (const auto& [key, value] : cgiEnv) {
            envStrings.push_back(key + "=" + value);
            envp.push_back(envStrings.back().data());
        }
        envp.push_back(nullptr);

        char* argv[] = {const_cast<char*>(scriptPath.c_str()), nullptr};

        // Remove debug statements to avoid interfering with CGI output

        execve(scriptPath.c_str(), argv, envp.data());
        perror("execve failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        close(inPipe[0]);
        close(outPipe[1]);

        write(inPipe[1], httpRequest.body.c_str(), httpRequest.body.size());
        close(inPipe[1]);

        std::string cgiOutput;
        captureCGIOutput(outPipe[0], cgiOutput);
        close(outPipe[0]);

        if (cgiOutput.empty()) {
            std::cerr << "CGI output is empty!" << std::endl;
        } else {
            std::cout << "CGI Output:\n" << cgiOutput << std::endl;
            write(clientSock, cgiOutput.c_str(), cgiOutput.size());
        }
    } else {
        std::cerr << "Fork failed!" << std::endl;
    }
}
