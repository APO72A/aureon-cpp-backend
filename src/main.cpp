#include <iostream>
#include <fstream>
#include <sstream>
#include "server/Platform.h"
#include "server/HttpResponse.h"

using aureon::HttpResponse;

// Reads a file from disk and returns it as an HttpResponse
// Returns a 404 response if the file cannot be opened.
HttpResponse serveFile(const std::string& path, const std::string& contentType) {
    std::ifstream file(path);

    if (!file) {
        return HttpResponse::notFound("404 - File not found");
    }

    std::cout << "Serving: " << path << std::endl;

    std::stringstream fileBuffer;
    fileBuffer << file.rdbuf();

    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.headers["Content-Type"] = contentType + "; charset=utf-8";
    response.body = fileBuffer.str();
    return response;
}

int main() {
    if (!aureon::platform::initNetworking()) {
        std::cerr << "Failed to initialise networking\n";
        return 1;
    }

    SocketType serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "AUREON server running on http://localhost:8080\n";

    while (true) {
        SocketType clientSocket = accept(serverSocket, nullptr, nullptr);

        char buffer[30000] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);

        std::string request(buffer);

        std::cout << "--- REQUEST ---\n" << request.substr(0, 200) << "\n---\n";

        HttpResponse response;

        if (request.find("GET /api/message") != std::string::npos) {
            response = HttpResponse::json(
                R"({"message": "Hello from the AUREON backend"})");
        }
        else if (request.find("GET /api/status") != std::string::npos) {
            response = HttpResponse::json(
                R"({"status": "online", "engine": "AUREON C++20"})");
        }
        else if (request.find("GET /api/greet") != std::string::npos) {
            std::string name = "Guest";

            std::size_t namePos = request.find("name=");
            if (namePos != std::string::npos) {
                namePos += 5;
                std::size_t nameEnd = request.find(' ', namePos);
                if (nameEnd != std::string::npos) {
                    name = request.substr(namePos, nameEnd - namePos);
                }
            }

            // name is untrusted input from URL, so it MUST be escaped
            // before being placed inside JSON
            std::string safeName = HttpResponse::escapeJson(name);
            response = HttpResponse::json(
                "{\"reply\": \"Welcome, " + safeName +
                ". AUREON backend received your request\"}");
        }
        else if (request.find("GET /style.css") != std::string::npos) {
            response = serveFile("frontend/style.css", "text/css");
        }
        else if (request.find("GET /script.js") != std::string::npos) {
            response = serveFile("frontend/script.js", "application/javascript");
        }
        else if (request.find("GET / ") != std::string::npos) {
            response = serveFile("frontend/index.html", "text/html");
        }
        else {
            response = HttpResponse::html("<h1>404 - Page Not Found</h1>", 404);
        }

        std::string raw = response.toRawString();
        send(clientSocket, raw.c_str(), raw.length(), 0);
        aureon::platform::closeSocket(clientSocket);
    }

    aureon::platform::closeSocket(serverSocket);
    aureon::platform::shutdownNetworking();
    return 0;

}