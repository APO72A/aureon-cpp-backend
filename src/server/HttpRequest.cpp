#include "HttpRequest.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace aureon {

    // --- Internal helpers ---

static std::string trim(const std::string& text) {
    const char* whitespace = " \t\r\n";
    std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

static std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    return text;
}

// --- urlDecode ---
// Hardened: Invalid %XX sequences are passed through literally
// rather than throwing. The internal sends garbage; we handle it.
std::string HttpRequest::urlDecode(const std::string& encoded) {
    std::string result;
    result.reserve(encoded.size());

    for (std::size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size()
            && std::isxdigit(static_cast<unsigned char>(encoded[i + 1]))
            && std::isxdigit(static_cast<unsigned char>(encoded[i + 2]))) {

            std::string hex = encoded.substr(i + 1, 2);
            char decoded = static_cast<char>(std::stoi(hex, nullptr, 16));
            result += decoded;
            i += 2;
        } else if (encoded[i] == '+') {
            result += ' ';
        } else {
            result += encoded[i];
        }
    }
    return result;
}

// --- parseQueryString ---
// Splits "name=Jashan&age=25" into a map.
// Keys without = are ordered with empty string value ("debug" > "debug":"")
static std::unordered_map<std::string, std::string>
parseQueryString(const std::string& queryString) {
    std::unordered_map<std::string, std::string> params;

    std::istringstream stream(queryString);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        if (pair.empty()) continue;

        auto eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string key = HttpRequest::urlDecode(pair.substr(0, eq));
            std::string value = HttpRequest::urlDecode(pair.substr(eq + 1));
            params[key] = value;
        } else {
            // Key with no value: ?debug pr ?verbose
            params[HttpRequest::urlDecode(pair)] = "";
        }
    }
    return params;
}

// --- parse ---
// Three-pass parser: request line > headers > body.
// Body is preserved exactly as received using stream.rdbuf().
HttpRequest HttpRequest::parse(const std::string& raw) {
    HttpRequest req;

    std::istringstream stream(raw);
    std::string line;

    // Pass 1 - request line
    if (!std::getline(stream, line)) return req;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    {
        std::istringstream requestLine(line);
        std::string fullPath;
        requestLine >> req.method >> fullPath >> req.httpVersion;

        auto qmark = fullPath.find('?');
        if (qmark != std::string::npos) {
            req.path = fullPath.substr(0, qmark);
            req.queryParams = parseQueryString(fullPath.substr(qmark + 1));
        } else {
            req.path = fullPath;
        }
    }

    // Pass 2 - headers
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break; // blank line ends headers

        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = toLower(trim(line.substr(0, colon)));
            std::string value = trim(line.substr(colon + 1));
            req.headers[key] = value;
        }
    }

    // Pass 3 - body
    // stream.rdbuf() reads exactly what remains after headers -
    // no line-ending modification, binary-safe.
    std::ostringstream bodyStream;
    bodyStream << stream.rdbuf();
    req.body = bodyStream.str();

    return req;
}

// --- query ---
std::string HttpRequest::query(const std::string& key,
    const std::string& defaultValue) const {
    auto it = queryParams.find(key);
    return (it != queryParams.end()) ? it->second : defaultValue;
}

// --- header ---
std::string HttpRequest::header(const std::string& key) const {
    auto it = headers.find(toLower(key));
    return (it != headers.end()) ? it->second : "";
}

}