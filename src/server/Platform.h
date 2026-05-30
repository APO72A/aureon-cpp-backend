#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketType = SOCKET;
constexpr SocketType INVALID_SOCKET_VALUE = INVALID_SOCKET;

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
using SocketType = int;
constexpr SocketType INVALID_SOCKET_VALUE = -1;
#endif

namespace aureon::platform {

    // Call once at program start
    // WSAStartup on Windows, no-op elsewhere.
    bool initNetworking();

    // Call once at program end.
    // WSACleanup on Windows. no-op elsewhere.
    void shutdownNetworking();

    // Close a socket using the correct OS call.
    void closeSocket(SocketType sock);
}