#include <iostream>
#include <fstream>
#include <sstream>
#include "server/Platform.h"
#include "server/HttpRequest.h"
#include "server/HttpResponse.h"
#include "server/Router.h"

using aureon::HttpRequest;
using aureon::HttpResponse;
using aureon::Router;

// Reads a file from disk and returns it as an HttpResponse.
HttpResponse serveFile(const std::string& path, const std::string& contentType) {
    std::ifstream file(path);
    if (!file) {
        return HttpResponse::notFound("404 - File not found");
    }
    std::stringstream fileBuffer;
    fileBuffer << file.rdbuf();

    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.headers["Content-Type"] = contentType + "; charset=utf-8";
    response.body = fileBuffer.str();
    return response;
}

int main () {
    if (!aureon::platform::initNetworking()) {
        std::cerr << "Failed to initialise networking\n";
        return 1;
    }

    // Register routes ---
    Router router;

    router.get("/api/status", [](const HttpRequest& req) {
        return HttpResponse::json(
            R"({"status": "online", "engine": "AUREON C++20"})");

    });

    router.get("/api/message", [](const HttpRequest& req) {
        return HttpResponse::json(
            R"({"message": "Hello from the AUREON backend"})");
    });

    router.get("/api/greet", [](const HttpRequest& req) {
        std::string name = req.query("name", "Guest");
        std::string safeName = HttpResponse::escapeJson(name);
        return HttpResponse::json(
            "{\"reply\": \"Welcome, " + safeName +
            ". AUREON backend received your request\"}");
    });

    router.get("/", [](const HttpRequest& req) {
        return serveFile("frontend/index.html", "text/html");
    });

    router.get("/style.css", [](const HttpRequest& req) {
        return serveFile("frontend/style.css", "text/css");
    });

    router.get("/script.js", [](const HttpRequest& req) {
        return serveFile("frontend/script.js", "application/javascript");
    });

    // --- Socket setup ---
    SocketType serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{}; // {} zero-initialises - fixes the linter warning
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "AUREON server running on http:://localhost:8080\n";

    // --- Accept loop ---
    while (true) {
        SocketType clientSocket = accept(serverSocket, nullptr, nullptr);

        char buffer[30000] = {0};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            aureon::platform::closeSocket(clientSocket);
            continue;
        }

        std::string raw(buffer,bytesReceived);

        HttpRequest req = HttpRequest::parse(raw);

        HttpResponse response = router.resolve(req);

        std::string rawResponse = response.toRawString();
        send(clientSocket, rawResponse.c_str(),
            static_cast<int>(rawResponse.length()), 0);

        aureon::platform::closeSocket(clientSocket);
    }

}