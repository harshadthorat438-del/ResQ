#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "../models/Operation.h"
#include "../structures/Stack.h"
#include "StorageService.h"

namespace services {

class HistoryService {
private:
    structures::Stack<models::Operation> recentStack; // In-memory LIFO Stack
    std::vector<models::Operation> persistentLog;     // Full chronological log
    int opIdCounter{1};

public:
    HistoryService() = default;

    /**
     * @brief Records an operation into the LIFO Stack and persistent log.
     */
    models::Operation recordOperation(const std::string& description,
                                      const std::string& category = "System",
                                      const std::string& relatedId = "") {
        models::Operation op;
        op.id = "OP-" + std::to_string(opIdCounter++);
        op.description = description;
        op.category = category.empty() ? "System" : category;
        op.relatedId = relatedId;
        op.timestamp = StorageService::getCurrentTimestamp();

        // Push to custom LIFO Stack (most recent becomes top)
        recentStack.push(op);
        // Also record to chronological log
        persistentLog.push_back(op);

        return op;
    }

    /**
     * @brief Retrieves recent operations from the C++ Stack in LIFO order (top = most recent).
     */
    std::vector<models::Operation> getRecentStackOperations() const {
        return recentStack.toVector();
    }

    /**
     * @brief Retrieves all operations from persistent historical log.
     */
    const std::vector<models::Operation>& getFullHistory() const {
        return persistentLog;
    }

    size_t getStackCount() const {
        return recentStack.size();
    }

    size_t getHistoryCount() const {
        return persistentLog.size();
    }

    void clear() {
        recentStack.clear();
        persistentLog.clear();
        opIdCounter = 1;
    }

    /**
     * @brief Saves persistent history to data/history.txt.
     * Format: id|description|category|relatedId|timestamp
     */
    bool saveToFile(const std::string& filepath = "data/history.txt") const {
        StorageService::ensureDirectory();
        std::ofstream out(filepath, std::ios::trunc);
        if (!out.is_open()) return false;

        for (const auto& op : persistentLog) {
            out << StorageService::sanitize(op.id) << "|"
                << StorageService::sanitize(op.description) << "|"
                << StorageService::sanitize(op.category) << "|"
                << StorageService::sanitize(op.relatedId) << "|"
                << StorageService::sanitize(op.timestamp) << "\n";
        }
        return true;
    }

    /**
     * @brief Loads history from data/history.txt and initializes Stack with recent records.
     */
    bool loadFromFile(const std::string& filepath = "data/history.txt") {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        clear();
        std::string line;
        int maxIdNum = 0;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto tokens = StorageService::split(line, '|');
            if (tokens.size() >= 5) {
                models::Operation op;
                op.id = tokens[0];
                op.description = tokens[1];
                op.category = tokens[2];
                op.relatedId = tokens[3];
                op.timestamp = tokens[4];

                if (op.id.rfind("OP-", 0) == 0) {
                    try {
                        int num = std::stoi(op.id.substr(3));
                        if (num > maxIdNum) maxIdNum = num;
                    } catch (...) {}
                }

                persistentLog.push_back(op);
                recentStack.push(op); // pushed in order, top becomes most recent
            }
        }
        opIdCounter = maxIdNum + 1;
        return true;
    }
};

} // namespace services
