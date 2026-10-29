#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include "../models/Resource.h"
#include "../models/Allocation.h"
#include "../models/RescueTeam.h"
#include "../structures/LinkedList.h"
#include "StorageService.h"

namespace services {

struct StockCheckResult {
    bool available{false};
    int currentStock{0};
    int requestedStock{0};
    int shortage{0};
    std::string unit{"units"};
};

class ResourceService {
private:
    // Core DSA: Linked List for dynamic inventory
    structures::LinkedList<models::Resource> inventory;

    // Core DSA: Linked List for rescue teams
    structures::LinkedList<models::RescueTeam> rescueTeams;

    std::vector<models::Allocation> allocations;
    int resIdCounter{1};
    int alcIdCounter{1};
    int teamIdCounter{1};

public:
    ResourceService() = default;

    /**
     * @brief Adds a new resource or replenishes existing stock in the Linked List.
     */
    models::Resource addOrReplenish(const std::string& name,
                                    const std::string& category,
                                    int quantity,
                                    int minThreshold = 20,
                                    const std::string& unit = "units",
                                    const std::string& customId = "") {
        if (name.empty()) {
            throw std::invalid_argument("Resource name cannot be empty.");
        }
        if (quantity <= 0) {
            throw std::invalid_argument("Quantity must be greater than zero.");
        }

        std::string lowerTarget = name;
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        models::Resource* existing = inventory.find([&lowerTarget](const models::Resource& r) {
            std::string curLower = r.name;
            std::transform(curLower.begin(), curLower.end(), curLower.begin(), ::tolower);
            return curLower == lowerTarget;
        });

        if (existing != nullptr) {
            existing->quantity += quantity;
            if (!category.empty()) existing->category = category;
            if (minThreshold > 0) existing->minThreshold = minThreshold;
            if (!unit.empty()) existing->unit = unit;
            return *existing;
        }

        // Insert new node in Linked List
        models::Resource res;
        if (!customId.empty()) {
            res.id = customId;
        } else {
            res.id = "RES-" + std::to_string(resIdCounter++);
        }
        res.name = name;
        res.category = category.empty() ? "General Supplies" : category;
        res.quantity = quantity;
        res.minThreshold = minThreshold > 0 ? minThreshold : 20;
        res.unit = unit.empty() ? "units" : unit;

        inventory.append(res);
        return res;
    }

    /**
     * @brief Adds a rescue team to the Rescue Teams Linked List.
     */
    models::RescueTeam addRescueTeam(const std::string& name,
                                     const std::string& type,
                                     int capacity = 6,
                                     const std::string& currentLocation = "Relief Camp",
                                     const std::string& customId = "",
                                     const std::string& equipment = "Standard Rescue Equipment") {
        models::RescueTeam team;
        if (!customId.empty()) {
            team.id = customId;
        } else {
            team.id = "T" + (teamIdCounter < 10 ? "0" + std::to_string(teamIdCounter) : std::to_string(teamIdCounter));
            teamIdCounter++;
        }
        team.name = name;
        team.type = type.empty() ? "General Rescue" : type;
        team.capacity = capacity > 0 ? capacity : 6;
        team.currentLocation = currentLocation.empty() ? "Relief Camp" : currentLocation;
        team.status = "Available";
        team.equipment = equipment;

        rescueTeams.append(team);
        return team;
    }

    /**
     * @brief Checks stock availability and calculates shortages without deducting stock.
     */
    StockCheckResult checkStock(const std::string& resourceName, int requestedQty) const {
        StockCheckResult res;
        res.requestedStock = requestedQty;
        std::string lowerTarget = resourceName;
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        const models::Resource* existing = inventory.find([&lowerTarget](const models::Resource& r) {
            std::string curLower = r.name;
            std::transform(curLower.begin(), curLower.end(), curLower.begin(), ::tolower);
            return curLower == lowerTarget;
        });

        if (existing == nullptr) {
            res.available = false;
            res.currentStock = 0;
            res.shortage = requestedQty;
            res.unit = "units";
            return res;
        }

        res.currentStock = existing->quantity;
        res.unit = existing->unit;
        if (existing->quantity >= requestedQty) {
            res.available = true;
            res.shortage = 0;
        } else {
            res.available = false;
            res.shortage = requestedQty - existing->quantity;
        }
        return res;
    }

    /**
     * @brief Allocates resources to a rescue request. Validates stock in Linked List.
     * Prevents allocation beyond available stock (anti-shortage safeguard).
     */
    models::Allocation allocate(const std::string& requestId,
                                const std::string& personName,
                                const std::string& location,
                                const std::string& resourceName,
                                int quantity,
                                const std::string& notes = "") {
        if (quantity <= 0) {
            throw std::invalid_argument("Allocation quantity must be greater than zero.");
        }

        std::string lowerTarget = resourceName;
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        models::Resource* res = inventory.find([&lowerTarget](const models::Resource& r) {
            std::string curLower = r.name;
            std::transform(curLower.begin(), curLower.end(), curLower.begin(), ::tolower);
            return curLower == lowerTarget;
        });

        if (res == nullptr) {
            throw std::runtime_error("Resource '" + resourceName + "' not found in inventory.");
        }

        if (res->quantity < quantity) {
            int shortage = quantity - res->quantity;
            throw std::runtime_error("Resource shortage: Requested " + std::to_string(quantity) +
                                     " " + res->unit + ", but only " + std::to_string(res->quantity) +
                                     " are currently available. Shortage: " + std::to_string(shortage) + " " + res->unit + ".");
        }

        // Deduct stock in Linked List node
        res->quantity -= quantity;

        // Create Allocation record
        models::Allocation alc;
        alc.id = "ALC-" + std::to_string(alcIdCounter++);
        alc.requestId = requestId;
        alc.personName = personName;
        alc.location = location;
        alc.resourceName = res->name;
        alc.quantity = quantity;
        alc.timestamp = StorageService::getCurrentTimestamp();
        alc.notes = notes;

        allocations.push_back(alc);
        return alc;
    }

    /**
     * @brief Reclaims or returns reusable resources (e.g. boats, medical units) upon completion/cancellation.
     */
    bool returnResource(const std::string& resourceName, int quantity) {
        if (quantity <= 0) return false;
        std::string lowerTarget = resourceName;
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        return inventory.update([&lowerTarget](const models::Resource& r) {
            std::string curLower = r.name;
            std::transform(curLower.begin(), curLower.end(), curLower.begin(), ::tolower);
            return curLower == lowerTarget;
        }, [quantity](models::Resource& r) {
            r.quantity += quantity;
        });
    }

    /**
     * @brief Assigns an available rescue team to an operation and sets status to Busy.
     * Prevents double-booking: fails if team is already Busy.
     */
    bool assignTeam(const std::string& teamId,
                    const std::string& targetZone,
                    const std::string& requestId,
                    double etaMinutes = 5.0) {
        models::RescueTeam* team = rescueTeams.find([&teamId](const models::RescueTeam& t) {
            return t.id == teamId;
        });

        if (team == nullptr) {
            throw std::runtime_error("Rescue team '" + teamId + "' not found.");
        }

        if (team->status == "Busy") {
            throw std::runtime_error("Team '" + team->name + " (" + team->id + ")' is already BUSY on another operation.");
        }

        team->status = "Busy";
        team->assignedZone = targetZone;
        team->assignedRequestId = requestId;
        team->etaMinutes = etaMinutes;
        return true;
    }

    /**
     * @brief Releases an assigned team back to Available status.
     */
    bool releaseTeam(const std::string& teamId, const std::string& newLocation = "") {
        return rescueTeams.update([&teamId](const models::RescueTeam& t) {
            return t.id == teamId;
        }, [&newLocation](models::RescueTeam& t) {
            t.status = "Available";
            if (!newLocation.empty()) {
                t.currentLocation = newLocation;
            }
            t.assignedZone = "";
            t.assignedRequestId = "";
            t.etaMinutes = 0.0;
        });
    }

    bool updateTeamStatus(const std::string& teamId, const std::string& newStatus, const std::string& newLocation = "") {
        return rescueTeams.update([&teamId](const models::RescueTeam& t) {
            return t.id == teamId;
        }, [&newStatus, &newLocation](models::RescueTeam& t) {
            t.status = newStatus;
            if (!newLocation.empty()) t.currentLocation = newLocation;
            if (newStatus == "Available") {
                t.assignedZone = "";
                t.assignedRequestId = "";
                t.etaMinutes = 0.0;
            }
        });
    }

    bool findTeam(const std::string& teamId, models::RescueTeam& outTeam) const {
        const models::RescueTeam* t = rescueTeams.find([&teamId](const models::RescueTeam& team) {
            return team.id == teamId;
        });
        if (t != nullptr) {
            outTeam = *t;
            return true;
        }
        return false;
    }

    std::vector<models::RescueTeam> getRescueTeams() const {
        return rescueTeams.toVector();
    }

    size_t getTeamCount() const {
        return rescueTeams.size();
    }

    size_t getAvailableTeamCount() const {
        size_t count = 0;
        std::vector<models::RescueTeam> teams = rescueTeams.toVector();
        for (const auto& t : teams) {
            if (t.status == "Available") count++;
        }
        return count;
    }

    size_t getBusyTeamCount() const {
        size_t count = 0;
        std::vector<models::RescueTeam> teams = rescueTeams.toVector();
        for (const auto& t : teams) {
            if (t.status == "Busy") count++;
        }
        return count;
    }

    std::vector<models::Resource> getInventory() const {
        return inventory.toVector();
    }

    const std::vector<models::Allocation>& getAllocations() const {
        return allocations;
    }

    void addExistingAllocation(const models::Allocation& alc) {
        allocations.push_back(alc);
    }

    size_t getResourceCount() const {
        return inventory.size();
    }

    int getTotalUnits() const {
        int total = 0;
        std::vector<models::Resource> inv = inventory.toVector();
        for (const auto& r : inv) {
            total += r.quantity;
        }
        return total;
    }

    size_t getLowStockCount() const {
        size_t count = 0;
        std::vector<models::Resource> inv = inventory.toVector();
        for (const auto& r : inv) {
            if (r.isLowStock()) {
                count++;
            }
        }
        return count;
    }

    void clear() {
        inventory.clear();
        rescueTeams.clear();
        allocations.clear();
        resIdCounter = 1;
        alcIdCounter = 1;
        teamIdCounter = 1;
    }

    /**
     * @brief Saves inventory to data/resources.txt.
     */
    bool saveResourcesToFile(const std::string& filepath = "data/resources.txt") const {
        StorageService::ensureDirectory();
        std::ofstream out(filepath, std::ios::trunc);
        if (!out.is_open()) return false;

        std::vector<models::Resource> inv = inventory.toVector();
        for (const auto& r : inv) {
            out << StorageService::sanitize(r.id) << "|"
                << StorageService::sanitize(r.name) << "|"
                << StorageService::sanitize(r.category) << "|"
                << r.quantity << "|"
                << r.minThreshold << "|"
                << StorageService::sanitize(r.unit) << "\n";
        }
        return true;
    }

    /**
     * @brief Loads resources from data/resources.txt.
     */
    bool loadResourcesFromFile(const std::string& filepath = "data/resources.txt") {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        inventory.clear();
        std::string line;
        int maxIdNum = 0;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto tokens = StorageService::split(line, '|');
            if (tokens.size() >= 6) {
                models::Resource r;
                r.id = tokens[0];
                r.name = tokens[1];
                r.category = tokens[2];
                try { r.quantity = std::stoi(tokens[3]); } catch (...) { r.quantity = 0; }
                try { r.minThreshold = std::stoi(tokens[4]); } catch (...) { r.minThreshold = 20; }
                r.unit = tokens[5];

                if (r.id.rfind("RES-", 0) == 0) {
                    try {
                        int num = std::stoi(r.id.substr(4));
                        if (num > maxIdNum) maxIdNum = num;
                    } catch (...) {}
                }

                inventory.append(r);
            }
        }
        resIdCounter = maxIdNum + 1;
        return true;
    }

    /**
     * @brief Saves rescue teams to data/teams.txt.
     */
    bool saveTeamsToFile(const std::string& filepath = "data/teams.txt") const {
        StorageService::ensureDirectory();
        std::ofstream out(filepath, std::ios::trunc);
        if (!out.is_open()) return false;

        std::vector<models::RescueTeam> list = rescueTeams.toVector();
        for (const auto& t : list) {
            out << StorageService::sanitize(t.id) << "|"
                << StorageService::sanitize(t.name) << "|"
                << StorageService::sanitize(t.type) << "|"
                << t.capacity << "|"
                << StorageService::sanitize(t.currentLocation) << "|"
                << StorageService::sanitize(t.status) << "|"
                << StorageService::sanitize(t.equipment) << "|"
                << StorageService::sanitize(t.assignedZone) << "|"
                << StorageService::sanitize(t.assignedRequestId) << "|"
                << t.etaMinutes << "\n";
        }
        return true;
    }

    /**
     * @brief Loads rescue teams from data/teams.txt.
     */
    bool loadTeamsFromFile(const std::string& filepath = "data/teams.txt") {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        rescueTeams.clear();
        std::string line;
        int maxIdNum = 0;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto tokens = StorageService::split(line, '|');
            if (tokens.size() >= 10) {
                models::RescueTeam t;
                t.id = tokens[0];
                t.name = tokens[1];
                t.type = tokens[2];
                try { t.capacity = std::stoi(tokens[3]); } catch (...) { t.capacity = 6; }
                t.currentLocation = tokens[4];
                t.status = tokens[5];
                t.equipment = tokens[6];
                t.assignedZone = tokens[7];
                t.assignedRequestId = tokens[8];
                try { t.etaMinutes = std::stod(tokens[9]); } catch (...) { t.etaMinutes = 0.0; }

                if (t.id.size() > 1 && (t.id[0] == 'T' || t.id[0] == 't')) {
                    try {
                        int num = std::stoi(t.id.substr(1));
                        if (num > maxIdNum) maxIdNum = num;
                    } catch (...) {}
                }

                rescueTeams.append(t);
            } else if (tokens.size() >= 9) {
                // Backward compatibility
                models::RescueTeam t;
                t.id = tokens[0];
                t.name = tokens[1];
                t.type = tokens[2];
                try { t.capacity = std::stoi(tokens[3]); } catch (...) { t.capacity = 6; }
                t.currentLocation = tokens[4];
                t.status = tokens[5];
                t.equipment = "Standard Equipment";
                t.assignedZone = tokens[6];
                t.assignedRequestId = tokens[7];
                try { t.etaMinutes = std::stod(tokens[8]); } catch (...) { t.etaMinutes = 0.0; }
                rescueTeams.append(t);
            }
        }
        teamIdCounter = maxIdNum + 1;
        return true;
    }

    /**
     * @brief Saves allocations to data/allocations.txt.
     */
    bool saveAllocationsToFile(const std::string& filepath = "data/allocations.txt") const {
        StorageService::ensureDirectory();
        std::ofstream out(filepath, std::ios::trunc);
        if (!out.is_open()) return false;

        for (const auto& a : allocations) {
            out << StorageService::sanitize(a.id) << "|"
                << StorageService::sanitize(a.requestId) << "|"
                << StorageService::sanitize(a.personName) << "|"
                << StorageService::sanitize(a.location) << "|"
                << StorageService::sanitize(a.resourceName) << "|"
                << a.quantity << "|"
                << StorageService::sanitize(a.timestamp) << "|"
                << StorageService::sanitize(a.notes) << "\n";
        }
        return true;
    }

    /**
     * @brief Loads allocations from data/allocations.txt.
     */
    bool loadAllocationsFromFile(const std::string& filepath = "data/allocations.txt") {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        allocations.clear();
        std::string line;
        int maxIdNum = 0;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto tokens = StorageService::split(line, '|');
            if (tokens.size() >= 8) {
                models::Allocation a;
                a.id = tokens[0];
                a.requestId = tokens[1];
                a.personName = tokens[2];
                a.location = tokens[3];
                a.resourceName = tokens[4];
                try { a.quantity = std::stoi(tokens[5]); } catch (...) { a.quantity = 0; }
                a.timestamp = tokens[6];
                a.notes = tokens[7];

                if (a.id.rfind("ALC-", 0) == 0) {
                    try {
                        int num = std::stoi(a.id.substr(4));
                        if (num > maxIdNum) maxIdNum = num;
                    } catch (...) {}
                }

                allocations.push_back(a);
            }
        }
        alcIdCounter = maxIdNum + 1;
        return true;
    }
};

} // namespace services
