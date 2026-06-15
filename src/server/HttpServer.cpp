#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <iostream>
#include <cstdint>
#include <cctype>
#include <chrono>

namespace aureon {

HttpServer::HttpServer(int port, const Router& router)
    : port(port), router(router), pool(4, 64) {}

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

void HttpServer::setRecvTimeout(SocketType sock, int seconds) {
#ifdef _WIN32
    // Windows takes a DWORD of milliseconds.
    DWORD timeout = static_cast<DWORD>(seconds * 1000);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<const char*>(&timeout), sizeof(timeout));

#else
    // POSIX takes a struct timeval.
    struct timeval tv{};
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif
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

bool HttpServer::readRequest(SocketType clientSocket, std::string& outRequest) {
    constexpr std::size_t MAX_REQUEST_SIZE = 1 * 1024 * 1024;
    constexpr std::size_t MAX_HEADER_SIZE = 16 * 1024;
    constexpr int RECV_TIMEOUT_SECONDS = 5; // Clock 1: per-recv idle limit
    constexpr int TOTAL_DEADLINE_SECONDS = 10; // Clock 2: whole-request limit

    // Clock 1: socket gives up if no bytes arrive within 5s of any single recv.
    setRecvTimeout(clientSocket, RECV_TIMEOUT_SECONDS);

    // Clock 2: the whole request must complete within 10s of starting.
    auto startTime = std::chrono::steady_clock::now();
    auto deadlineExceeded = [&]() {
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        return elapsed > std::chrono::seconds(TOTAL_DEADLINE_SECONDS);
    };

    std::string raw;
    char buffer[4096];
    std::size_t headerEnd = std::string::npos;

    // -- Phase A: read until full header block --
    while (headerEnd == std::string::npos) {
        if (deadlineExceeded()) return false; // Clock 2 catches the drip-feeder

        int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) return false; // Clock 1 fired, disconnect, or error

        raw.append(buffer, static_cast<std::size_t>(bytes));
        if (raw.size() > MAX_HEADER_SIZE) return false;

        headerEnd = raw.find("\r\n\r\n");
    }

    std::size_t bodyStart = headerEnd + 4;

    std::size_t contentLength = 0;
    {
        std::string headerBlock = raw.substr(0, headerEnd);
        std::string lower = headerBlock;
        for (char& c : lower) c = static_cast<char>(std::tolower(
            static_cast<unsigned char>(c)));

        auto pos = lower.find("content-length:");
        if (pos != std::string::npos) {
            std::size_t valueStart = pos + std::string("content-length:").size();
            std::size_t lineEnd = headerBlock.find("\r\n", valueStart);
            std::string value = headerBlock.substr(
                valueStart,
                (lineEnd == std::string::npos ? headerBlock.size() : lineEnd) - valueStart);
            try {
                long parsed = std::stol(value);
                if (parsed < 0) return false;
                contentLength = static_cast<std::size_t>(parsed);
            } catch (...) {
                return false;
            }
        }
    }

    if (bodyStart + contentLength > MAX_REQUEST_SIZE) return false;

    // --- Phase C: read body, both clocks still enforced ---
    while (raw.size() - bodyStart < contentLength) {
        if (deadlineExceeded()) return false; // Clock 2 again, for slow bodies

        int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) return false;

        raw.append(buffer, static_cast<std::size_t>(bytes));
        if (raw.size() > MAX_REQUEST_SIZE) return false;
    }

    outRequest = std::move(raw);
    return true;
}

void HttpServer::handleClient(SocketType clientSocket) {
    std::string raw;
    if (!readRequest(clientSocket, raw)) {
        // Malformed, too large, or disconnected - refuse cleanly.
        HttpResponse bad = HttpResponse::text("400 Bad Request", 400);
        sendAll(clientSocket, bad.toRawString());
        aureon::platform::closeSocket(clientSocket);
        return;
    }

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
        // Hand the socket to a worker. Ownership transfers to the job -
        // the loop does not close it; handleClient does.
        bool accepted = pool.submit([this, clientSocket]() {
            handleClient(clientSocket);
        });

        if (!accepted) {
            // Queue full or shutting down - reject cleanly
            // Ownership never transferred, so WE close it.
            // send a small fixed response directly (bounded work, no routing,
            // no request parsing), then close. We do NOT call handleClient.
            HttpResponse busy = HttpResponse::serviceUnavailable();
            sendAll(clientSocket, busy.toRawString());
            aureon::platform::closeSocket(clientSocket);
        }

    }
}

}