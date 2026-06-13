#include "HttpResponse.h"
#include <sstream>

namespace aureon {

    std::string HttpResponse::toRawString() const {
        std::ostringstream out;

        // Status line - built from a parameter, never retyped per route.
        out << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";

        // Content-Length computed here, always. Handlers can't forget it.
        out << "Content-Length: " << body.size() << "\r\n";

        for (const auto& [key, value] : headers) {
            out << key << ": " << value << "\r\n";
        }

        out << "\r\n"; // blank line separates headers from body
        out << body;
        return out.str();
    }

    HttpResponse HttpResponse::json(const std::string& jsonBody, int status) {
        HttpResponse r;
        r.statusCode = status;
        r.statusText = (status == 200) ? "OK" : "Error";
        r.headers["Content-Type"] = "application/json; charset=utf-8";
        r.headers["Access-Control-Allow-Origin"] = "*";
        r.body = jsonBody;
        return r;
    }

    HttpResponse HttpResponse::html(const std::string& htmlBody, int status) {
        HttpResponse r;
        r.statusCode = status;
        r.statusText = (status == 200) ? "OK" : "Error";
        r.headers["Content-Type"] = "text/html; charset=utf-8";
        r.body = htmlBody;
        return r;
    }

    HttpResponse HttpResponse::text(const std::string& textBody, int status) {
        HttpResponse r;
        r.statusCode = status;
        r.statusText = (status == 200) ? "OK" : "Error";
        r.headers["Content-Type"] = "text/plain; charset=utf-8";
        r.body = textBody;
        return r;
    }

    HttpResponse HttpResponse::notFound(const std::string& message) {
        HttpResponse r;
        r.statusCode = 404;
        r.statusText = "Not Found";
        r.headers["Content-Type"] = "text/plain; charset=utf-8";
        r.body = message;
        return r;
    }

    HttpResponse HttpResponse::serviceUnavailable(const std::string& message) {
        HttpResponse r;
        r.statusCode = 503;
        r.statusText = "Service Unavailable";
        r.headers["Content-Type"] = "text/plain; charset=utf-8";
        // Tell well-behaved clients to back off and retry.
        r.headers["Retry-After"] = "1";
        r.body = message;
        return r;
    }

    std::string HttpResponse::escapeJson(const std::string& raw) {
        std::string out;
        out.reserve(raw.size());
        for (char c : raw) {
            switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\";break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out += c;       break;
            }
        }
        return out;
    }
}