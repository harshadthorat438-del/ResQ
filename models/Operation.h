#pragma once

#include <string>
#include "../third_party/json.hpp"

namespace models {

struct Operation {
    std::string id;
    std::string description;
    std::string category; // "Request", "Resource", "Allocation", "Location", "Road", "System"
    std::string relatedId;
    std::string timestamp;

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"description", description},
            {"category", category},
            {"relatedId", relatedId},
            {"timestamp", timestamp}
        };
    }

    static Operation fromJson(const nlohmann::json& j) {
        Operation op;
        if (j.contains("id")) op.id = j["id"].get<std::string>();
        if (j.contains("description")) op.description = j["description"].get<std::string>();
        if (j.contains("category")) op.category = j["category"].get<std::string>();
        if (j.contains("relatedId")) op.relatedId = j["relatedId"].get<std::string>();
        if (j.contains("timestamp")) op.timestamp = j["timestamp"].get<std::string>();
        return op;
    }
};

} // namespace models
