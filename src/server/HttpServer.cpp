#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <iostream>
#include <cstdint>

namespace aureon {

HttpServer::HttpServer(int port, const Router& router)
    : port(port), router(router) {}

bool HttpServer::setupSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET_VALUE) {
        std::cerr << "Failed to create socket\n";
        return false;
    }

    // Allow the port to be reused immediately after the server restarts,
    // instead of waiting for the OS to release it.
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind to port " << port << "\n";
        return false;
    }

    if (listen(serverSocket, 16) < 0) {
        std::cerr << "Failed to listen on port " << port << "\n";
        return false;
    }

    return true;
}

bool HttpServer::sendAll(SocketType sock, const std::string& data) {
    std::size_t totalSent = 0;
    while (totalSent < data.size()) {
        int sent = send(sock,
            data.c_str() + totalSent,
            static_cast<int>(data.size() - totalSent),
            0);
        if (sent <= 0) {
            return false; // client disconnected or error
        }
        totalSent += static_cast<std::size_t>(sent);
    }
    return true;
}

void HttpServer::handleClient(SocketType clientSocket) {
    char buffer[30000] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
        aureon::platform::closeSocket(clientSocket);
        return;
    }

    std::string raw(buffer, bytesReceived);

    HttpRequest req = HttpRequest::parse(raw);
    HttpResponse response = router.resolve(req);

    sendAll(clientSocket, response.toRawString());
    aureon::platform::closeSocket(clientSocket);
}

bool HttpServer::start() {
    if (!setupSocket()) {
        return false;
    }

    std::cout << "AUREON server running on http://localhost:" << port << "\n";

    while (true) {
        SocketType clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET_VALUE) {
            continue; // accept failed for this connection, keep serving others
        }
        handleClient(clientSocket);
    }
}

}