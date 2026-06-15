#include "server/Platform.h"
#include "server/HttpRequest.h"
#include "server/HttpResponse.h"
#include "server/Router.h"
#include "server/HttpServer.h"
#include "fstream"
#include "sstream"


using aureon::HttpRequest;
using aureon::HttpResponse;
using aureon::Router;
using aureon::HttpServer;

// Reads a file from disk and returns it as an HttpResponse.
static HttpResponse serveFile(const std::string& path,
    const std::string& contentType) {
    std::ifstream file(path);
    if (!file) {
        return HttpResponse::notFound("404 - File not found");
    }
    std::stringstream fileBuffer;
    fileBuffer << file.rdbuf();

    HttpResponse response;
    response.headers["Content-Type"] = contentType + "; charset=utf-8";
    response.body = fileBuffer.str();
    return response;
}

int main() {
    if (!aureon::platform::initNetworking()) {
        return 1;
    }

    Router router;

    router.get("/api/status", [](const HttpRequest&) {
        return HttpResponse::json(
            R"({"status": "online", "engine": "AUREON C++20"})");
    });


    router.get("/api/message", [](const HttpRequest&) {
        return HttpResponse::json(
            R"({"message": "Hello from the AUREON backend"})");
    });

    router.get("/api/greet", [](const HttpRequest& req) {
        std::string safeName = HttpResponse::escapeJson(req.query("name", "Guest"));
        return HttpResponse::json(
            "{\"reply\": \"Welcome, " + safeName +
            ". AUREON backend received your request\"}");
    });


    router.get("/", [](const HttpRequest&) {
        return serveFile("frontend/index.html", "text/html");
    });

    router.get("/style.css", [](const HttpRequest&) {
        return serveFile("frontend/style.css", "text/css");
    });

    router.get("/script.js", [](const HttpRequest&) {
        return serveFile("frontend/script.js", "application/javascript");
    });

    HttpServer server(8080, router);
    if (!server.start()) {
        return 1;
    }

    aureon::platform::shutdownNetworking();
    return 0;
}

