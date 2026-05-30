#pragma once
#include <string>
#include <map>

// HttpResponse - owns HTTP transport formatting for AUREON.
// Handlers describe INTENT (json, html, notFound). This class turns
// that into a correct raw HTTP response: status line, headers,
// Content-Length, body. Handlers never touch protocol text.

namespace aureon {

    class HttpResponse {
    public:
        int statusCode = 200;
        std::string statusText = "OK";
        std::map<std::string, std::string> headers;
        std::string body;

        // Serialise into a complete raw HTTP response string.
        // Content-Length is computed here, always, from body.size().
        std::string toRawString() const;

        // --- Factory helpers: the vocabulary handlers actually use ---
        static HttpResponse json(const std::string& jsonBody, int status = 200);
        static HttpResponse html(const std::string& htmlBody, int status = 200);
        static HttpResponse text(const std::string& textBody, int status = 200);
        static HttpResponse notFound(const std::string& message = "404 - Not Found");

        // Escape a string so it is safe to embed inside a JSON string value.
        // This is the fix for the /api/greet injection problem.
        static std::string escapeJson(const std::string& raw);
    };

}