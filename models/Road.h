#pragma once

#include <string>
#include "../third_party/json.hpp"

namespace models {

struct Road {
    std::string id;
    std::string from;
    std::string to;
    double distance{1.0};
    std::string status{"Open"}; // "Open", "Hazardous", "Blocked"

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"from", from},
            {"to", to},
            {"distance", distance},
            {"status", status}
        };
    }

    static Road fromJson(const nlohmann::json& j) {
        Road r;
        if (j.contains("id")) r.id = j["id"].get<std::string>();
        if (j.contains("from")) r.from = j["from"].get<std::string>();
        if (j.contains("to")) r.to = j["to"].get<std::string>();
        if (j.contains("distance")) r.distance = j["distance"].get<double>();
        if (j.contains("status")) r.status = j["status"].get<std::string>();
        return r;
    }
};

} // namespace models
