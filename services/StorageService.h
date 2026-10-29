#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <algorithm>

namespace services {

class StorageService {
public:
    static std::string getDataDir() {
        return "data";
    }

    static void ensureDirectory() {
        try {
            std::filesystem::path p(getDataDir());
            if (!std::filesystem::exists(p)) {
                std::filesystem::create_directories(p);
            }
        } catch (...) {}
    }

    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // Helper: Split string by delimiter
    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Helper: Sanitize text to avoid delimiter conflicts
    static std::string sanitize(std::string text) {
        std::replace(text.begin(), text.end(), '|', ' ');
        std::replace(text.begin(), text.end(), '\n', ' ');
        std::replace(text.begin(), text.end(), '\r', ' ');
        return text;
    }
};

} // namespace services
