#pragma once

#include <string>
#include "../third_party/json.hpp"

namespace models {

/**
 * @struct RescueTeam
 * @brief Represents an operational rescue responder team or medical dispatch squad.
 * 
 * Fields mandated by Module 4:
 * - Team ID (e.g. T01, T02, T03)
 * - Team Name
 * - Team Capability / Type (General Rescue, Medical Team, Flood Rescue, etc.)
 * - Current Location
 * - Availability Status: "Available" or "Busy"
 * - Equipment / Supported emergency types
 * - Responders / Victims Capacity
 * - Assigned Request ID & ETA (minutes)
 */
struct RescueTeam {
    std::string id;
    std::string name;
    std::string type{"General Rescue"}; // "General Rescue", "Medical Team", "Flood Rescue"
    int capacity{6};
    std::string currentLocation{"Relief Camp"};
    std::string status{"Available"};    // "Available", "Busy"
    std::string equipment{"Standard Rescue Gear"};
    std::string assignedZone;
    std::string assignedRequestId;
    double etaMinutes{0.0};

    bool isAvailable() const {
        return status == "Available";
    }

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"name", name},
            {"type", type},
            {"capacity", capacity},
            {"currentLocation", currentLocation},
            {"status", status},
            {"equipment", equipment},
            {"assignedZone", assignedZone},
            {"assignedRequestId", assignedRequestId},
            {"etaMinutes", etaMinutes},
            {"isAvailable", isAvailable()}
        };
    }

    static RescueTeam fromJson(const nlohmann::json& j) {
        RescueTeam t;
        if (j.contains("id")) t.id = j["id"].get<std::string>();
        if (j.contains("name")) t.name = j["name"].get<std::string>();
        if (j.contains("type")) t.type = j["type"].get<std::string>();
        if (j.contains("capacity")) t.capacity = j["capacity"].get<int>();
        if (j.contains("currentLocation")) t.currentLocation = j["currentLocation"].get<std::string>();
        if (j.contains("status")) t.status = j["status"].get<std::string>();
        if (j.contains("equipment")) t.equipment = j["equipment"].get<std::string>();
        if (j.contains("assignedZone")) t.assignedZone = j["assignedZone"].get<std::string>();
        if (j.contains("assignedRequestId")) t.assignedRequestId = j["assignedRequestId"].get<std::string>();
        if (j.contains("etaMinutes")) t.etaMinutes = j["etaMinutes"].get<double>();
        return t;
    }
};

} // namespace models
