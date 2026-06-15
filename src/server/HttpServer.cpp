#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <iostream>
#include <cstdint>
#include <cctype>

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
    // Safety caps - the internet sends malformed nonsense; we refuse to
    // grow without limit. These are the guardrails against a hostile client.
    constexpr std::size_t MAX_REQUEST_SIZE = 1 * 1024 * 1024; // 1 MB total
    constexpr std::size_t MAX_HEADER_SIZE = 16 * 1024; // 16 KB of headers

    std::string raw;
    char buffer[4096];
    std::size_t headerEnd = std::string::npos;

    // --- Phase A: read until we have the full header block (\r\n\r\n) ---
    while (headerEnd == std::string::npos) {
        int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            return false; // client disconnected or error before headers done
        }
        raw.append(buffer, static_cast<std::size_t>(bytes));

        if (raw.size() > MAX_HEADER_SIZE) {
            return false; // headers too large - refuse
        }

        headerEnd = raw.find("\r\n\r\n");
    }

    // --- Phase B: figure out how much body to expect ---
    std::size_t bodyStart = headerEnd + 4; // skip past the \r\n\r\n

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
                if (parsed < 0) return false; // negative length = malformed
                contentLength = static_cast<std::size_t>(parsed);
            } catch (...) {
                return false; // Content-Length present but not a number
            }
        }
        // No Content-Length header > contentLength stays 0 (GET, etc.)
    }

    // Reject oversized bodies beforer reading them.
    if (bodyStart + contentLength > MAX_REQUEST_SIZE) {
        return false;
    }

    // --- Phase C: keep reading until we have the full body ---
    while (raw.size() - bodyStart < contentLength) {
        int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            return false; // client disconnected mid-body
        }
        raw.append(buffer, static_cast<std::size_t>(bytes));

        if (raw.size() > MAX_REQUEST_SIZE) {
            return false; // total request too large
        }
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