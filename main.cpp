#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#ifdef _WIN32
using SocketType = SOCKET;
#else
using SocketType = int;
#endif

void closeSocket(SocketType socket) {
    #ifdef _WIN32
        closesocket(socket);
    #else
        close(socket);
    #endif
}

std::string serveFile(const std::string& path, const std::string& contentType) {
    std::ifstream file(path);

    if (!file) {
        return
         "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n\r\n"
            "404 - File not found";

    }

    std::cout << "Serving: " << path << std::endl;

    std::stringstream fileBuffer;
    fileBuffer << file.rdbuf();

    return
       "HTTP/1.1 200 OK\r\n"
       "Content-Type: " + contentType + "; charset=utf-8\r\n\r\n" +
       fileBuffer.str();
}

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif

    SocketType serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "Server running on http://localhost:8080\n";

    while (true) {
        SocketType clientSocket = accept(serverSocket, nullptr, nullptr);

        char buffer[30000] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);

        std::string request(buffer);
        std::cout << "Request:\n" << request << "\n";

        std::string response;

        if (request.find("GET /api/message") != std::string::npos) {
            response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=utf-8\r\n\r\n"
                "{\"message\": \"Hello from the C++ backend 🚀\"}";
        }
        else if (request.find("GET /api/status") != std::string::npos) {
            response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=utf-8\r\n\r\n"
                "{\"status\": \"online\", \"engine\": \"C++\", \"message\": \"Server is running 🔥\"}";
        }

        else if (request.find("GET /api/greet") != std::string::npos) {
            std::string name = "Guest";

            std::size_t namePos = request.find("name=");
            if (namePos != std::string::npos) {
                namePos += 5;

                std::size_t nameEnd = request.find(" ", namePos);
                if (nameEnd != std::string::npos) {
                    name = request.substr(namePos, nameEnd - namePos);
                }
            }

            response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=utf-8\r\n\r\n"
                "{\"reply\": \"Welcome, " + name + ". AUREON backend received your request ⚙️\"}";
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
            response =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html; charset=utf-8\r\n\r\n"
                "<h1>404 - Page Not Found</h1>";
        }


        send(clientSocket, response.c_str(), response.length(), 0);
        closeSocket(clientSocket);
    }

    closeSocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}