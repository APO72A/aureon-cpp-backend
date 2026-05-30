#include "Platform.h"

namespace aureon::platform {

    bool initNetworking() {
#ifdef _WIN32
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
        return true;
        #endif
    }

    void shutdownNetworking() {
        #ifdef _WIN32
        WSACleanup();
        #endif
    }

    void closeSocket(SocketType sock) {
        #ifdef _WIN32
        closesocket(sock);
        #else
        close(sock);
        #endif
    }
}