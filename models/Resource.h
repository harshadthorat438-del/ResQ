#pragma once

#include <string>
#include "../third_party/json.hpp"

namespace models {

struct Resource {
    std::string id;
    std::string name;
    std::string category;
    int quantity{0};
    int minThreshold{25};
    std::string unit{"units"};

    bool isLowStock() const {
        return quantity <= minThreshold;
    }

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"name", name},
            {"category", category},
            {"quantity", quantity},
            {"minThreshold", minThreshold},
            {"unit", unit},
            {"isLowStock", isLowStock()}
        };
    }

    static Resource fromJson(const nlohmann::json& j) {
        Resource r;
        if (j.contains("id")) r.id = j["id"].get<std::string>();
        if (j.contains("name")) r.name = j["name"].get<std::string>();
        if (j.contains("category")) r.category = j["category"].get<std::string>();
        if (j.contains("quantity")) r.quantity = j["quantity"].get<int>();
        if (j.contains("minThreshold")) r.minThreshold = j["minThreshold"].get<int>();
        if (j.contains("unit")) r.unit = j["unit"].get<std::string>();
        return r;
    }
};

} // namespace models
