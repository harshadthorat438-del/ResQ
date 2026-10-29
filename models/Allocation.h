#pragma once

#include <string>
#include "../third_party/json.hpp"

namespace models {

struct Allocation {
    std::string id;
    std::string requestId;
    std::string personName;
    std::string location;
    std::string resourceName;
    int quantity{0};
    std::string timestamp;
    std::string notes;

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"requestId", requestId},
            {"personName", personName},
            {"location", location},
            {"resourceName", resourceName},
            {"quantity", quantity},
            {"timestamp", timestamp},
            {"notes", notes}
        };
    }

    static Allocation fromJson(const nlohmann::json& j) {
        Allocation a;
        if (j.contains("id")) a.id = j["id"].get<std::string>();
        if (j.contains("requestId")) a.requestId = j["requestId"].get<std::string>();
        if (j.contains("personName")) a.personName = j["personName"].get<std::string>();
        if (j.contains("location")) a.location = j["location"].get<std::string>();
        if (j.contains("resourceName")) a.resourceName = j["resourceName"].get<std::string>();
        if (j.contains("quantity")) a.quantity = j["quantity"].get<int>();
        if (j.contains("timestamp")) a.timestamp = j["timestamp"].get<std::string>();
        if (j.contains("notes")) a.notes = j["notes"].get<std::string>();
        return a;
    }
};

} // namespace models
