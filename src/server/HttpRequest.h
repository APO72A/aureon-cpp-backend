#pragma once

#include <string>
#include <unordered_map>

namespace aureon {

    class HttpRequest {
        public:
        bool valid = false;

        std::string method;
        std::string path;
        std::string httpVersion;
        std::string body;

        std::unordered_map<std::string, std::string> queryParams;
        std::unordered_map<std::string, std::string> headers;

        static HttpRequest parse(const std::string& raw);

        std::string query(const std::string& key,
                          const std::string& defaultValue = "") const;

        std::string header(const std::string& key) const;

        static std::string urlDecode(const std::string& encoded);
    };
}