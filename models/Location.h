#pragma once

#include <string>
#include "../third_party/json.hpp"

namespace models {

struct Location {
    std::string id;
    std::string name;
    std::string type; // "Relief Camp", "Hospital", "Village", "Rescue Center", "Bridge Checkpoint", "Shelter"
    double x{0.0};
    double y{0.0};

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"name", name},
            {"type", type},
            {"x", x},
            {"y", y}
        };
    }

    static Location fromJson(const nlohmann::json& j) {
        Location loc;
        if (j.contains("id")) loc.id = j["id"].get<std::string>();
        if (j.contains("name")) loc.name = j["name"].get<std::string>();
        if (j.contains("type")) loc.type = j["type"].get<std::string>();
        if (j.contains("x")) loc.x = j["x"].get<double>();
        if (j.contains("y")) loc.y = j["y"].get<double>();
        return loc;
    }
};

} // namespace models
