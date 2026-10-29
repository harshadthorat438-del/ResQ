#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "../models/Request.h"
#include "../structures/LinkedList.h"
#include "../structures/Queue.h"
#include "../structures/PriorityQueue.h"
#include "../structures/Sorting.h"
#include "StorageService.h"

namespace services {

struct PriorityReportItem {
    models::Request report;
    double urgencyScore;
    int rank;
};

class RescueService {
private:
    // Core DSA 1: Singly Linked List for all disaster reports
    structures::LinkedList<models::Request> reportList;

    // Core DSA 2: Binary Max-Heap Priority Queue for urgency triage
    structures::PriorityQueue<models::Request> priorityHeap;

    // Core DSA 3: FIFO Queue for dispatch scheduling
    structures::Queue<models::Request> dispatchQueue;

    int idCounter{101};

public:
    RescueService() = default;

    /**
     * @brief Computes documented urgency score using composite triage policy.
     */
    static double calculateUrgencyScore(const models::Request& req) {
        return req.calculateUrgencyScore();
    }

    /**
     * @brief Rebuilds the binary max-heap and dispatch queue from active (uncompleted) reports.
     */
    void rebuildPriorityStructures() {
        priorityHeap.clear();
        dispatchQueue.clear();

        std::vector<models::Request> all = reportList.toVector();
        
        // Filter active reports (not Completed and not Cancelled)
        std::vector<models::Request> active;
        for (const auto& r : all) {
            if (r.status != "Completed" && r.status != "Cancelled") {
                active.push_back(r);
            }
        }

        // Insert into Binary Max-Heap
        for (const auto& r : active) {
            priorityHeap.insert(r, r.calculateUrgencyScore());
        }

        // Sort active reports descending by urgency using custom MergeSort
        structures::Sorting::mergeSort(active, [](const models::Request& a, const models::Request& b) {
            return a.calculateUrgencyScore() > b.calculateUrgencyScore();
        });

        // Enqueue into Dispatch Queue in prioritized rank order
        for (const auto& r : active) {
            dispatchQueue.enqueue(r);
        }
    }

    /**
     * @brief Registers a new disaster report and stores it in the Singly Linked List.
     */
    models::Request addReport(const std::string& personName,
                              const std::string& location,
                              const std::string& emergencyType,
                              int affectedPeople,
                              int immediateRescueCount,
                              const std::string& requiredResourceType = "Medical Kits",
                              int requiredResourceQty = 1,
                              const std::string& requiredCapability = "General Rescue",
                              const std::string& priority = "Medium",
                              const std::string& description = "",
                              const std::string& customId = "") {
        models::Request req;
        if (!customId.empty()) {
            req.id = customId;
        } else {
            req.id = "R" + std::to_string(idCounter++);
        }
        req.personName = personName;
        req.location = location;
        req.emergencyType = emergencyType.empty() ? "Flood" : emergencyType;
        req.affectedPeople = std::max(1, affectedPeople);
        req.immediateRescueCount = std::max(0, immediateRescueCount);
        req.requiredResourceType = requiredResourceType.empty() ? "Medical Kits" : requiredResourceType;
        req.requiredResourceQty = std::max(1, requiredResourceQty);
        req.requiredCapability = requiredCapability.empty() ? "General Rescue" : requiredCapability;
        req.priority = priority.empty() ? "Medium" : priority;
        req.description = description;
        req.status = "Reported";
        req.assignedTeamId = "";
        req.escalated = false;
        req.timestamp = StorageService::getCurrentTimestamp();

        // 1. Insert into Singly Linked List
        reportList.append(req);

        // 2. Synchronize Priority Queue (Max-Heap) and Dispatch Queue
        rebuildPriorityStructures();

        return req;
    }

    /**
     * @brief Searches for a report by ID using O(N) traversal in Singly Linked List.
     */
    bool findReport(const std::string& id, models::Request& outReport) const {
        const models::Request* ptr = reportList.find([&id](const models::Request& r) {
            return r.id == id;
        });
        if (ptr != nullptr) {
            outReport = *ptr;
            return true;
        }
        return false;
    }

    /**
     * @brief Updates status and assigned team of a report in the Singly Linked List.
     */
    bool updateReportStatus(const std::string& id, const std::string& newStatus, const std::string& teamId = "") {
        bool ok = reportList.update([&id](const models::Request& r) {
            return r.id == id;
        }, [&newStatus, &teamId](models::Request& r) {
            r.status = newStatus;
            if (!teamId.empty()) {
                r.assignedTeamId = teamId;
            }
        });

        if (ok) {
            rebuildPriorityStructures();
        }
        return ok;
    }

    /**
     * @brief Escalates emergency urgency (operator manual override).
     */
    bool escalateReport(const std::string& id) {
        bool ok = reportList.update([&id](const models::Request& r) {
            return r.id == id;
        }, [](models::Request& r) {
            r.escalated = true;
            if (r.priority != "Critical") {
                r.priority = "Critical";
            }
        });

        if (ok) {
            rebuildPriorityStructures();
        }
        return ok;
    }

    /**
     * @brief Dequeues and processes the next pending dispatch from the Priority-Ranked Queue.
     */
    models::Request dispatchNextPriority() {
        if (dispatchQueue.isEmpty()) {
            throw std::runtime_error("No active emergency reports available for dispatch.");
        }
        models::Request nextReq = dispatchQueue.dequeue();
        
        // Update status in Singly Linked List to Prioritized / In Progress
        updateReportStatus(nextReq.id, "Prioritized");
        return nextReq;
    }

    /**
     * @brief Extracts highest-priority emergency using Binary Max-Heap extractMax().
     */
    models::Request extractHighestUrgency() {
        if (priorityHeap.isEmpty()) {
            throw std::runtime_error("Priority Queue is empty. No pending emergencies to triage.");
        }
        models::Request top = priorityHeap.extractMax();
        updateReportStatus(top.id, "Prioritized");
        return top;
    }

    /**
     * @brief Returns reports sorted by urgency using manual MergeSort.
     */
    std::vector<PriorityReportItem> getPriorityRankedReports() const {
        std::vector<models::Request> all = reportList.toVector();
        std::vector<PriorityReportItem> items;

        for (const auto& r : all) {
            if (r.status != "Completed" && r.status != "Cancelled") {
                items.push_back({r, r.calculateUrgencyScore(), 0});
            }
        }

        // Custom FDS MergeSort
        structures::Sorting::mergeSort(items, [](const PriorityReportItem& a, const PriorityReportItem& b) {
            return a.urgencyScore > b.urgencyScore;
        });

        for (size_t i = 0; i < items.size(); ++i) {
            items[i].rank = static_cast<int>(i + 1);
        }

        return items;
    }

    /**
     * @brief Returns reports sorted by affected population using MergeSort.
     */
    std::vector<models::Request> getReportsSortedByPopulation() const {
        std::vector<models::Request> list = reportList.toVector();
        structures::Sorting::mergeSort(list, [](const models::Request& a, const models::Request& b) {
            return a.affectedPeople > b.affectedPeople;
        });
        return list;
    }

    /**
     * @brief Returns all reports from Singly Linked List.
     */
    std::vector<models::Request> getAllReports() const {
        return reportList.toVector();
    }

    // Dashboard Statistics Metrics
    size_t getTotalReportsCount() const {
        return reportList.size();
    }

    size_t getCriticalAndHighCount() const {
        size_t count = 0;
        std::vector<models::Request> all = reportList.toVector();
        for (const auto& r : all) {
            if ((r.priority == "Critical" || r.priority == "High") && r.status != "Completed") {
                count++;
            }
        }
        return count;
    }

    size_t getPendingCount() const {
        size_t count = 0;
        std::vector<models::Request> all = reportList.toVector();
        for (const auto& r : all) {
            if (r.status == "Reported" || r.status == "Prioritized" || r.status == "Pending") {
                count++;
            }
        }
        return count;
    }

    size_t getActiveOperationsCount() const {
        size_t count = 0;
        std::vector<models::Request> all = reportList.toVector();
        for (const auto& r : all) {
            if (r.status == "Assigned" || r.status == "In Progress") {
                count++;
            }
        }
        return count;
    }

    size_t getCompletedCount() const {
        size_t count = 0;
        std::vector<models::Request> all = reportList.toVector();
        for (const auto& r : all) {
            if (r.status == "Completed" || r.status == "Processed") {
                count++;
            }
        }
        return count;
    }

    int getTotalAffectedPeople() const {
        int total = 0;
        std::vector<models::Request> all = reportList.toVector();
        for (const auto& r : all) {
            if (r.status != "Completed" && r.status != "Cancelled") {
                total += r.affectedPeople;
            }
        }
        return total;
    }

    int getTotalImmediateRescueCount() const {
        int total = 0;
        std::vector<models::Request> all = reportList.toVector();
        for (const auto& r : all) {
            if (r.status != "Completed" && r.status != "Cancelled") {
                total += r.immediateRescueCount;
            }
        }
        return total;
    }

    void clear() {
        reportList.clear();
        priorityHeap.clear();
        dispatchQueue.clear();
        idCounter = 101;
    }

    /**
     * @brief Saves reports to data/requests.txt.
     */
    bool saveToFile(const std::string& filepath = "data/requests.txt") const {
        StorageService::ensureDirectory();
        std::ofstream out(filepath, std::ios::trunc);
        if (!out.is_open()) return false;

        std::vector<models::Request> all = reportList.toVector();
        for (const auto& r : all) {
            out << StorageService::sanitize(r.id) << "|"
                << StorageService::sanitize(r.personName) << "|"
                << StorageService::sanitize(r.location) << "|"
                << StorageService::sanitize(r.emergencyType) << "|"
                << r.affectedPeople << "|"
                << r.immediateRescueCount << "|"
                << StorageService::sanitize(r.requiredResourceType) << "|"
                << r.requiredResourceQty << "|"
                << StorageService::sanitize(r.requiredCapability) << "|"
                << StorageService::sanitize(r.priority) << "|"
                << StorageService::sanitize(r.description) << "|"
                << StorageService::sanitize(r.status) << "|"
                << StorageService::sanitize(r.assignedTeamId) << "|"
                << (r.escalated ? "1" : "0") << "|"
                << StorageService::sanitize(r.timestamp) << "\n";
        }
        return true;
    }

    /**
     * @brief Loads reports from data/requests.txt into Singly Linked List.
     */
    bool loadFromFile(const std::string& filepath = "data/requests.txt") {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        reportList.clear();
        std::string line;
        int maxId = 100;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto tokens = StorageService::split(line, '|');
            if (tokens.size() >= 15) {
                models::Request r;
                r.id = tokens[0];
                r.personName = tokens[1];
                r.location = tokens[2];
                r.emergencyType = tokens[3];
                try { r.affectedPeople = std::stoi(tokens[4]); } catch (...) { r.affectedPeople = 1; }
                try { r.immediateRescueCount = std::stoi(tokens[5]); } catch (...) { r.immediateRescueCount = 0; }
                r.requiredResourceType = tokens[6];
                try { r.requiredResourceQty = std::stoi(tokens[7]); } catch (...) { r.requiredResourceQty = 1; }
                r.requiredCapability = tokens[8];
                r.priority = tokens[9];
                r.description = tokens[10];
                r.status = tokens[11];
                r.assignedTeamId = tokens[12];
                r.escalated = (tokens[13] == "1" || tokens[13] == "true");
                r.timestamp = tokens[14];

                // Track ID numbering
                if (r.id.size() > 1 && (r.id[0] == 'R' || r.id[0] == 'r')) {
                    try {
                        int num = std::stoi(r.id.substr(1));
                        if (num > maxId) maxId = num;
                    } catch (...) {}
                }

                reportList.append(r);
            } else if (tokens.size() >= 9) {
                // Backward compatibility with legacy 9-field format
                models::Request r;
                r.id = tokens[0];
                r.personName = tokens[1];
                r.location = tokens[2];
                r.emergencyType = tokens[3];
                try { r.affectedPeople = std::stoi(tokens[4]); } catch (...) { r.affectedPeople = 1; }
                r.priority = tokens[5];
                r.description = tokens[6];
                r.status = tokens[7];
                r.timestamp = tokens[8];
                r.immediateRescueCount = std::max(0, r.affectedPeople / 2);
                r.requiredResourceType = "Medical Kits";
                r.requiredResourceQty = 5;
                r.requiredCapability = "General Rescue";
                r.assignedTeamId = "";
                r.escalated = false;

                reportList.append(r);
            }
        }

        idCounter = maxId + 1;
        rebuildPriorityStructures();
        return true;
    }
};

} // namespace services
