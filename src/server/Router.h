#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "HttpRequest.h"
#include "HttpResponse.h"

namespace aureon {

// A route key method + path. Both must match exactly.
struct RouteKey {
    std::string method;
    std::string path;

    bool operator==(const RouteKey& other) const {
        return method == other.method && path == other.path;
    }
};

// Hash function so RouteKey works as an unordered_map key.
struct RouteKeyHash {
    std::size_t operator()(const RouteKey& k) const {
        return std::hash<std::string>{}(k.method + "|" + k.path);
    }
};

// A Handler is any callable that takes a request and returns a response.
using Handler = std::function<HttpResponse(const HttpRequest&)>;

class Router {
public:
    // Register a GET route.
    void get(const std::string& path, Handler handler);

    // Register a POST route.
    void post(const std::string& path, Handler handler);

    // Resolve an incoming request to a handler and call it.
    // Returns 404 if no matching route exists.
    HttpResponse resolve(const HttpRequest& req) const;

private:
    std::unordered_map<RouteKey, Handler, RouteKeyHash> routes;

    // Internal: register any method + path combination.
    void addRoute(const std::string& method,
        const std::string& path,
        Handler handler);
};

}