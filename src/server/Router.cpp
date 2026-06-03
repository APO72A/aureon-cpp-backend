#include "Router.h"

namespace aureon {

void Router::addRoute(const std::string& method,
    const std::string& path,
    Handler handler) {
    routes[{method, path}] = std::move(handler);
}

void Router::get(const std::string& path, Handler handler) {
    addRoute("GET", path, std::move(handler));
}

void Router::post(const std::string& path, Handler handler) {
    addRoute("POST", path, std::move(handler));
}

HttpResponse Router::resolve(const HttpRequest& req) const {
    // Reject malformed requests before even looking up a route.
    if (!req.valid) {
        return HttpResponse::text("404 Bad Request", 400);
    }

    auto it = routes.find({req.method, req.path});
    if (it != routes.end()) {
        return it->second(req); // call the handler
    }

    return HttpResponse::notFound("404 - Route not found");
}

}