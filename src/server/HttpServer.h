#pragma once
#include <string>
#include "Platform.h"
#include "Router.h"
#include "../core/ThreadPool.h"

namespace aureon {

class HttpServer {
public:
    // Construct with a port and a router.
    // The router is stored by reference - the server uses the
    // same router main.cpp built, it does not own or copy it.
    HttpServer(int port, const Router& router);

    // Start the accept loop. Blocks forever (until the process is killed)
    // Returns false if the server failed to start (socket/bind/listen error).
    bool start();

    // No copying - the server owns a thread pool, which cannot be copied.
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

private:
    int port;
    const Router& router;
    SocketType serverSocket = INVALID_SOCKET_VALUE;
    ThreadPool pool;

    // Set up the listening socket. Returns false on any failure.
    bool setupSocket();

    // Handle one accepted client connection start-to-finish.
    void handleClient(SocketType clientSocket);

    // Read a complete HTTP request from the socket: full headers, then
    // exactly Content-Length body bytes. Returns false if the request is
    // malformed, too large, or the client disconnects early.
    bool readRequest(SocketType clientSocket, std::string& outRequest);

    // Send an entire buffer, looping until all bytes are sent.
    // Replaces the raw single send() - this is your sendAll().
    bool sendAll(SocketType sock, const std::string& data);
};
}