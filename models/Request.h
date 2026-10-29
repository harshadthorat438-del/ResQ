#pragma once

#include <string>
#include <algorithm>
#include "../third_party/json.hpp"

namespace models {

/**
 * @struct Request
 * @brief Represents a field disaster incident / rescue report.
 * 
 * Fields mandated by Module 2 & 8:
 * - Unique Report ID (e.g. R101, REQ-1)
 * - Reporter / Contact Name
 * - Disaster Location (Relief Camp, Village A, B, C, etc.)
 * - Emergency Disaster Type (Flood, Fire, Landslide, Earthquake, Medical Emergency, Other)
 * - Severity Level (Critical, High, Medium, Low)
 * - Affected People Count
 * - Immediate Rescue Required Count
 * - Required Resource Type & Quantity (e.g., Medical Kits: 15, Rescue Boats: 2)
 * - Required Team Capability (General Rescue, Medical Team, Flood Rescue)
 * - Operational Status: Reported -> Prioritized -> Assigned -> In Progress -> Completed (or Cancelled)
 * - Assigned Team ID
 * - Escalated Flag
 * - Timestamp
 */
struct Request {
    std::string id;
    std::string personName;
    std::string location;
    std::string emergencyType{"Flood"};
    int affectedPeople{1};
    int immediateRescueCount{0};
    std::string requiredResourceType{"Medical Kits"};
    int requiredResourceQty{1};
    std::string requiredCapability{"General Rescue"};
    std::string priority{"Medium"}; // "Critical", "High", "Medium", "Low"
    std::string description;
    std::string status{"Reported"}; // "Reported", "Prioritized", "Assigned", "In Progress", "Completed", "Cancelled"
    std::string assignedTeamId;
    bool escalated{false};
    std::string timestamp;

    /**
     * @brief Computes composite urgency score based on:
     * - Immediate threat to life (emergency type)
     * - Severity level
     * - People requiring immediate rescue
     * - Total affected population
     * - Operator escalation boost
     */
    double calculateUrgencyScore() const {
        double score = 0.0;

        // 1. Severity Base Score
        if (priority == "Critical") {
            score += 100.0;
        } else if (priority == "High") {
            score += 60.0;
        } else if (priority == "Medium") {
            score += 30.0;
        } else {
            score += 10.0;
        }

        // 2. People Requiring Immediate Rescue (High weight)
        score += static_cast<double>(immediateRescueCount) * 15.0;

        // 3. Hazard / Threat to Life based on disaster type
        if (emergencyType == "Earthquake" || emergencyType == "Flood") {
            score += 30.0;
        } else if (emergencyType == "Fire" || emergencyType == "Landslide") {
            score += 25.0;
        } else if (emergencyType == "Medical Emergency") {
            score += 20.0;
        } else {
            score += 10.0;
        }

        // 4. Total Affected Population factor
        score += static_cast<double>(affectedPeople) * 2.0;

        // 5. Operator Escalation boost
        if (escalated) {
            score += 60.0;
        }

        return score;
    }

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"personName", personName},
            {"location", location},
            {"emergencyType", emergencyType},
            {"affectedPeople", affectedPeople},
            {"immediateRescueCount", immediateRescueCount},
            {"requiredResourceType", requiredResourceType},
            {"requiredResourceQty", requiredResourceQty},
            {"requiredCapability", requiredCapability},
            {"priority", priority},
            {"description", description},
            {"status", status},
            {"assignedTeamId", assignedTeamId},
            {"escalated", escalated},
            {"urgencyScore", calculateUrgencyScore()},
            {"timestamp", timestamp}
        };
    }

    static Request fromJson(const nlohmann::json& j) {
        Request r;
        if (j.contains("id")) r.id = j["id"].get<std::string>();
        if (j.contains("personName")) r.personName = j["personName"].get<std::string>();
        if (j.contains("location")) r.location = j["location"].get<std::string>();
        if (j.contains("emergencyType")) r.emergencyType = j["emergencyType"].get<std::string>();
        if (j.contains("affectedPeople")) r.affectedPeople = j["affectedPeople"].get<int>();
        if (j.contains("immediateRescueCount")) r.immediateRescueCount = j["immediateRescueCount"].get<int>();
        if (j.contains("requiredResourceType")) r.requiredResourceType = j["requiredResourceType"].get<std::string>();
        if (j.contains("requiredResourceQty")) r.requiredResourceQty = j["requiredResourceQty"].get<int>();
        if (j.contains("requiredCapability")) r.requiredCapability = j["requiredCapability"].get<std::string>();
        if (j.contains("priority")) r.priority = j["priority"].get<std::string>();
        if (j.contains("description")) r.description = j["description"].get<std::string>();
        if (j.contains("status")) r.status = j["status"].get<std::string>();
        if (j.contains("assignedTeamId")) r.assignedTeamId = j["assignedTeamId"].get<std::string>();
        if (j.contains("escalated")) r.escalated = j["escalated"].get<bool>();
        if (j.contains("timestamp")) r.timestamp = j["timestamp"].get<std::string>();
        return r;
    }
};

} // namespace models
