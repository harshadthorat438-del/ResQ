#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include "../services/RescueService.h"
#include "../services/ResourceService.h"
#include "../services/NetworkService.h"
#include "../services/HistoryService.h"
#include "../services/DatabaseService.h"

namespace api {

using json = nlohmann::json;

class ApiServer {
private:
    httplib::Server server;
    services::RescueService rescueService;
    services::ResourceService resourceService;
    services::NetworkService networkService;
    services::HistoryService historyService;
    mutable std::mutex dataMutex;
    int port;

public:
    explicit ApiServer(int p = 8080) : port(p) {
        setupRoutes();
    }

    void loadPersistedData() {
        std::lock_guard<std::mutex> lock(dataMutex);
        services::StorageService::ensureDirectory();

        auto& db = services::DatabaseService::getInstance();
        db.initializeSchema();

        if (!db.isDatabaseEmpty()) {
            std::cout << "[PERSISTENCE] Restoring state from SQLite relational database (data/disaster_system.db)...\n";
            rescueService.clear();
            resourceService.clear();
            networkService.clear();
            historyService.clear();

            // 1. Locations
            for (const auto& loc : db.loadAllLocations()) {
                networkService.addLocation(loc.name, loc.type, loc.x, loc.y, loc.id);
            }
            // 2. Roads
            for (const auto& rd : db.loadAllRoads()) {
                networkService.addRoad(rd.from, rd.to, rd.distance, rd.status, rd.id);
            }
            // 3. Teams
            for (const auto& tm : db.loadAllTeams()) {
                resourceService.addRescueTeam(tm.name, tm.type, tm.capacity, tm.currentLocation, tm.id, tm.equipment);
                if (tm.status == "Busy") {
                    resourceService.assignTeam(tm.id, tm.assignedZone, tm.assignedRequestId, tm.etaMinutes);
                }
            }
            // 4. Resources
            for (const auto& res : db.loadAllResources()) {
                resourceService.addOrReplenish(res.name, res.category, res.quantity, res.minThreshold, res.unit, res.id);
            }
            // 5. Reports
            for (const auto& rep : db.loadAllReports()) {
                rescueService.addReport(rep.personName, rep.location, rep.emergencyType, rep.affectedPeople,
                                       rep.immediateRescueCount, rep.requiredResourceType, rep.requiredResourceQty,
                                       rep.requiredCapability, rep.priority, rep.description, rep.id);
                if (rep.status != "Reported" && rep.status != "Pending") {
                    rescueService.updateReportStatus(rep.id, rep.status, rep.assignedTeamId);
                }
                if (rep.escalated) {
                    rescueService.escalateReport(rep.id);
                }
            }
            // 6. Allocations
            for (const auto& alc : db.loadAllAllocations()) {
                resourceService.addExistingAllocation(alc);
            }
            // 7. History
            for (const auto& h : db.loadAllHistory()) {
                historyService.recordOperation(h.description, h.category, h.relatedId);
            }
            std::cout << "[PERSISTENCE] Successfully loaded from SQLite: Reports=" << rescueService.getTotalReportsCount()
                      << ", Teams=" << resourceService.getTeamCount()
                      << ", Resources=" << resourceService.getResourceCount()
                      << ", Locations=" << networkService.getLocationCount()
                      << ", Roads=" << networkService.getRoadCount() << "\n";
        } else {
            std::cout << "[INFO] SQLite database is empty. Seeding Section 8 initial demonstration data...\n";
            seedDemoDataInternal();
            saveAllDataInternal();
        }
    }

    /**
     * @brief Seeds initial demonstration data strictly complying with Prompt Section 8.
     */
    void seedDemoDataInternal() {
        auto& db = services::DatabaseService::getInstance();
        db.clearAllData();

        rescueService.clear();
        resourceService.clear();
        networkService.clear();
        historyService.clear();

        // -------------------------------------------------------------
        // 1. Locations (Graph Vertices)
        // -------------------------------------------------------------
        auto l1 = networkService.addLocation("Relief Camp", "Rescue Station", 120.0, 250.0, "LOC-1");
        auto l2 = networkService.addLocation("Village A", "Affected Area", 330.0, 140.0, "LOC-2");
        auto l3 = networkService.addLocation("Village B", "Affected Area", 330.0, 360.0, "LOC-3");
        auto l4 = networkService.addLocation("Village C", "Affected Area", 580.0, 140.0, "LOC-4");
        auto l5 = networkService.addLocation("Medical Center", "Rescue Station", 780.0, 250.0, "LOC-5");

        db.insertLocation(l1);
        db.insertLocation(l2);
        db.insertLocation(l3);
        db.insertLocation(l4);
        db.insertLocation(l5);

        // -------------------------------------------------------------
        // 2. Roads (Bidirectional Edges with Travel Times in Minutes)
        // -------------------------------------------------------------
        auto rd1 = networkService.addRoad("Relief Camp", "Village A", 5.0, "Open", "RD-1");
        auto rd2 = networkService.addRoad("Village A", "Village B", 4.0, "Open", "RD-2");
        auto rd3 = networkService.addRoad("Village A", "Village C", 7.0, "Open", "RD-3");
        auto rd4 = networkService.addRoad("Village B", "Medical Center", 3.0, "Open", "RD-4");
        auto rd5 = networkService.addRoad("Village C", "Medical Center", 6.0, "Open", "RD-5");

        db.insertRoad(rd1);
        db.insertRoad(rd2);
        db.insertRoad(rd3);
        db.insertRoad(rd4);
        db.insertRoad(rd5);

        // -------------------------------------------------------------
        // 3. Rescue Teams (Linked List)
        // -------------------------------------------------------------
        auto t1 = resourceService.addRescueTeam("General Rescue Alpha", "General Rescue", 10, "Relief Camp", "T01", "Extrication tools, First Aid, Ropes");
        auto t2 = resourceService.addRescueTeam("Medical Team Beta", "Medical Team", 6, "Medical Center", "T02", "Trauma kits, Defibrillators, Mobile triage");
        auto t3 = resourceService.addRescueTeam("Flood Rescue Gamma", "Flood Rescue", 8, "Village A", "T03", "Inflatable boats, Life jackets, Dive gear");

        db.insertTeam(t1);
        db.insertTeam(t2);
        db.insertTeam(t3);

        // -------------------------------------------------------------
        // 4. Resources (Linked List)
        // -------------------------------------------------------------
        auto res1 = resourceService.addOrReplenish("Medical Kits", "Medical", 50, 20, "kits", "RES-1");
        auto res2 = resourceService.addOrReplenish("Food Packets", "Food", 80, 30, "packets", "RES-2");
        auto res3 = resourceService.addOrReplenish("Water Bottles", "Water", 100, 40, "bottles", "RES-3");
        auto res4 = resourceService.addOrReplenish("Rescue Boats", "Equipment", 5, 2, "boats", "RES-4");

        db.insertOrUpdateResource(res1);
        db.insertOrUpdateResource(res2);
        db.insertOrUpdateResource(res3);
        db.insertOrUpdateResource(res4);

        // -------------------------------------------------------------
        // 5. Sample Reports (Singly Linked List, Priority Queue, Queue)
        // -------------------------------------------------------------
        models::Request r1 = rescueService.addReport(
            "Commander Sharma", "Village A", "Earthquake", 12, 8,
            "Medical Kits", 15, "General Rescue", "Critical",
            "People trapped in a building following structural collapse; 8 casualties requiring immediate extraction.",
            "R101"
        );

        models::Request r2 = rescueService.addReport(
            "Sarpanch Patil", "Village B", "Flood", 25, 10,
            "Rescue Boats", 2, "Flood Rescue", "High",
            "Water entering homes rapidly due to canal breach; 10 residents stranded on rooftops.",
            "R102"
        );

        models::Request r3 = rescueService.addReport(
            "Dr. Rao", "Village C", "Medical Emergency", 3, 2,
            "Medical Kits", 5, "Medical Team", "Critical",
            "Severe medical emergency following building damage; 2 patients requiring critical stabilization.",
            "R103"
        );

        db.insertReport(r1);
        db.insertReport(r2);
        db.insertReport(r3);

        // -------------------------------------------------------------
        // 6. Operation History (Stack)
        // -------------------------------------------------------------
        auto h1 = historyService.recordOperation("System initialized with Section 8 demonstration road network", "System");
        auto h2 = historyService.recordOperation("Rescue teams T01, T02, T03 deployed to operational stations", "Resource");
        auto h3 = historyService.recordOperation("Relief supplies inventory registered: Medical Kits (50), Food Packets (80), Water (100), Boats (5)", "Resource");
        auto h4 = historyService.recordOperation("Emergency Report R101 registered: Village A (Critical, 8 immediate rescue)", "Request", r1.id);
        auto h5 = historyService.recordOperation("Emergency Report R102 registered: Village B (High, 10 immediate rescue)", "Request", r2.id);
        auto h6 = historyService.recordOperation("Emergency Report R103 registered: Village C (Critical, 2 immediate rescue)", "Request", r3.id);

        db.insertHistory(h1);
        db.insertHistory(h2);
        db.insertHistory(h3);
        db.insertHistory(h4);
        db.insertHistory(h5);
        db.insertHistory(h6);
    }

    bool saveAllDataInternal() {
        auto& db = services::DatabaseService::getInstance();
        for (const auto& l : networkService.getLocations()) db.insertLocation(l);
        for (const auto& rd : networkService.getRoads()) db.insertRoad(rd);
        for (const auto& t : resourceService.getRescueTeams()) db.insertTeam(t);
        for (const auto& res : resourceService.getInventory()) db.insertOrUpdateResource(res);
        for (const auto& rep : rescueService.getAllReports()) db.insertReport(rep);
        for (const auto& alc : resourceService.getAllocations()) db.insertAllocation(alc);
        for (const auto& h : historyService.getFullHistory()) db.insertHistory(h);

        bool r1 = rescueService.saveToFile("data/requests.txt");
        bool r2 = resourceService.saveResourcesToFile("data/resources.txt");
        bool r3 = resourceService.saveAllocationsToFile("data/allocations.txt");
        bool r4 = networkService.saveLocationsToFile("data/locations.txt");
        bool r5 = networkService.saveRoadsToFile("data/roads.txt");
        bool r6 = historyService.saveToFile("data/history.txt");
        bool r7 = resourceService.saveTeamsToFile("data/teams.txt");
        return r1 && r2 && r3 && r4 && r5 && r6 && r7;
    }

    void setupRoutes() {
        server.set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
        });

        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.status = 204;
        });

        server.set_mount_point("/", "./frontend");

        server.Get("/", [](const httplib::Request&, httplib::Response& res) {
            std::ifstream file("frontend/index.html");
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                res.set_content(buffer.str(), "text/html");
            } else {
                res.status = 404;
                res.set_content("<h1>404: Frontend index.html not found</h1>", "text/html");
            }
        });

        // Health
        server.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
            json resp = {
                {"status", "online"},
                {"service", "Disaster Rescue & Management System"},
                {"version", "2.5.0 (DSA Course Project Edition)"},
                {"compiler", "C++17 (MinGW-w64 GCC)"}
            };
            res.set_content(resp.dump(), "application/json");
        });

        // ========================================================
        // 1. Dashboard Statistics
        // ========================================================
        server.Get("/api/stats", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            json resp = {
                {"success", true},
                {"totalReports", rescueService.getTotalReportsCount()},
                {"criticalAndHighReports", rescueService.getCriticalAndHighCount()},
                {"pendingRequests", rescueService.getPendingCount()},
                {"activeOperations", rescueService.getActiveOperationsCount()},
                {"completedOperations", rescueService.getCompletedCount()},
                {"availableTeams", resourceService.getAvailableTeamCount()},
                {"busyTeams", resourceService.getBusyTeamCount()},
                {"totalResourceUnits", resourceService.getTotalUnits()},
                {"lowStockCount", resourceService.getLowStockCount()},
                {"locationCount", networkService.getLocationCount()},
                {"roadCount", networkService.getRoadCount()},
                {"blockedRoadCount", networkService.getBlockedRoadCount()},
                {"totalAffectedPeople", rescueService.getTotalAffectedPeople()},
                {"totalImmediateRescue", rescueService.getTotalImmediateRescueCount()},
                {"recentOperationsCount", historyService.getStackCount()}
            };
            res.set_content(resp.dump(), "application/json");
        });

        // ========================================================
        // 2. Disaster Reports Management (Singly Linked List)
        // ========================================================
        server.Get("/api/reports", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            std::string statusFilter = req.has_param("status") ? req.get_param_value("status") : "";
            std::string severityFilter = req.has_param("severity") ? req.get_param_value("severity") : "";
            std::string search = req.has_param("search") ? req.get_param_value("search") : "";
            std::string sortBy = req.has_param("sortBy") ? req.get_param_value("sortBy") : "";

            std::vector<models::Request> list;
            if (sortBy == "population") {
                list = rescueService.getReportsSortedByPopulation();
            } else {
                list = rescueService.getAllReports();
            }

            // Lowercase search term
            std::string searchLower = search;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

            json reportsArray = json::array();
            for (const auto& r : list) {
                if (!statusFilter.empty() && statusFilter != "all" && r.status != statusFilter) {
                    continue;
                }
                if (!severityFilter.empty() && severityFilter != "all" && r.priority != severityFilter) {
                    continue;
                }
                if (!searchLower.empty()) {
                    std::string locLower = r.location;
                    std::transform(locLower.begin(), locLower.end(), locLower.begin(), ::tolower);
                    std::string nameLower = r.personName;
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                    std::string idLower = r.id;
                    std::transform(idLower.begin(), idLower.end(), idLower.begin(), ::tolower);

                    if (locLower.find(searchLower) == std::string::npos &&
                        nameLower.find(searchLower) == std::string::npos &&
                        idLower.find(searchLower) == std::string::npos) {
                        continue;
                    }
                }
                reportsArray.push_back(r.toJson());
            }

            res.set_content(json({
                {"success", true},
                {"count", reportsArray.size()},
                {"reports", reportsArray}
            }).dump(), "application/json");
        });

        server.Post("/api/reports", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string personName = body.value("personName", "");
                std::string location = body.value("location", "");
                std::string emergencyType = body.value("emergencyType", "Flood");
                int affectedPeople = body.value("affectedPeople", 1);
                int immediateRescueCount = body.value("immediateRescueCount", 0);
                std::string requiredResourceType = body.value("requiredResourceType", "Medical Kits");
                int requiredResourceQty = body.value("requiredResourceQty", 1);
                std::string requiredCapability = body.value("requiredCapability", "General Rescue");
                std::string priority = body.value("priority", "Medium");
                std::string description = body.value("description", "");

                if (personName.empty() || location.empty()) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "Reporter name and disaster location are required."}}).dump(), "application/json");
                    return;
                }

                models::Request r = rescueService.addReport(
                    personName, location, emergencyType, affectedPeople, immediateRescueCount,
                    requiredResourceType, requiredResourceQty, requiredCapability, priority, description
                );

                auto op = historyService.recordOperation("Report #" + r.id + " registered: " + location + " (" + priority + ", " + emergencyType + ")", "Request", r.id);

                // Persist to SQLite
                services::DatabaseService::getInstance().insertReport(r);
                services::DatabaseService::getInstance().insertHistory(op);

                res.status = 201;
                res.set_content(json({{"success", true}, {"report", r.toJson()}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // Escalate Report Urgency
        server.Post("/api/reports/escalate", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string id = body.value("id", "");
                if (id.empty()) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "Report ID is required."}}).dump(), "application/json");
                    return;
                }

                bool ok = rescueService.escalateReport(id);
                if (ok) {
                    auto op = historyService.recordOperation("Urgency escalated for Report #" + id + " to CRITICAL (+priority boost)", "Request", id);
                    services::DatabaseService::getInstance().escalateReport(id);
                    services::DatabaseService::getInstance().insertHistory(op);

                    models::Request updated;
                    rescueService.findReport(id, updated);
                    res.set_content(json({{"success", true}, {"message", "Report escalated successfully."}, {"report", updated.toJson()}}).dump(), "application/json");
                } else {
                    res.status = 404;
                    res.set_content(json({{"success", false}, {"message", "Report not found."}}).dump(), "application/json");
                }
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // ========================================================
        // 3. Emergency Prioritization (Binary Heap Priority Queue)
        // ========================================================
        server.Get("/api/priority-queue", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            auto rankedItems = rescueService.getPriorityRankedReports();

            json rankings = json::array();
            for (const auto& item : rankedItems) {
                json itemJson = item.report.toJson();
                itemJson["urgencyScore"] = item.urgencyScore;
                itemJson["priorityRank"] = item.rank;
                rankings.push_back(itemJson);
            }

            res.set_content(json({
                {"success", true},
                {"count", rankings.size()},
                {"prioritizedReports", rankings}
            }).dump(), "application/json");
        });

        // Triage Extract Max (Binary Max-Heap)
        server.Post("/api/priority-queue/triage-next", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                models::Request highest = rescueService.extractHighestUrgency();
                auto op = historyService.recordOperation("Binary Heap Triage: Dispatched highest-urgency emergency #" + highest.id + " at " + highest.location, "Request", highest.id);
                services::DatabaseService::getInstance().updateReportStatus(highest.id, "Prioritized");
                services::DatabaseService::getInstance().insertHistory(op);

                res.set_content(json({
                    {"success", true},
                    {"algorithm", "Binary Max-Heap extractMax()"},
                    {"message", "Extracted highest-urgency emergency #" + highest.id + " (" + highest.location + ")"},
                    {"dispatched", highest.toJson()}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // FIFO Queue Dispatch
        server.Post("/api/priority-queue/fifo-next", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                models::Request nextReq = rescueService.dispatchNextPriority();
                auto op = historyService.recordOperation("Queue FIFO Dispatch: #" + nextReq.id + " at " + nextReq.location, "Request", nextReq.id);
                services::DatabaseService::getInstance().updateReportStatus(nextReq.id, "Prioritized");
                services::DatabaseService::getInstance().insertHistory(op);

                res.set_content(json({
                    {"success", true},
                    {"algorithm", "Custom Queue dequeue()"},
                    {"message", "Dequeued emergency #" + nextReq.id + " in rank order."},
                    {"dispatched", nextReq.toJson()}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // ========================================================
        // 4. Rescue Team Management (Linked List)
        // ========================================================
        server.Get("/api/teams", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            auto teams = resourceService.getRescueTeams();
            json arr = json::array();
            for (const auto& t : teams) arr.push_back(t.toJson());

            res.set_content(json({
                {"success", true},
                {"count", arr.size()},
                {"teams", arr}
            }).dump(), "application/json");
        });

        server.Post("/api/teams", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string name = body.value("name", "");
                std::string type = body.value("type", "General Rescue");
                int capacity = body.value("capacity", 6);
                std::string currentLocation = body.value("currentLocation", "Relief Camp");
                std::string equipment = body.value("equipment", "Standard Gear");

                if (name.empty()) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "Team name is required."}}).dump(), "application/json");
                    return;
                }

                models::RescueTeam t = resourceService.addRescueTeam(name, type, capacity, currentLocation, "", equipment);
                auto op = historyService.recordOperation("Rescue Team " + t.id + " (" + t.name + ") added to fleet", "Resource", t.id);

                // Persist to SQLite
                services::DatabaseService::getInstance().insertTeam(t);
                services::DatabaseService::getInstance().insertHistory(op);

                res.status = 201;
                res.set_content(json({{"success", true}, {"team", t.toJson()}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        server.Post("/api/teams/release", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string teamId = body.value("teamId", "");
                std::string newLocation = body.value("newLocation", "Relief Camp");

                bool ok = resourceService.releaseTeam(teamId, newLocation);
                if (ok) {
                    auto op = historyService.recordOperation("Team " + teamId + " released to Available status at " + newLocation, "Resource", teamId);
                    services::DatabaseService::getInstance().updateTeamStatus(teamId, "Available", "", newLocation);
                    services::DatabaseService::getInstance().insertHistory(op);

                    res.set_content(json({{"success", true}, {"message", "Team released to Available status."}}).dump(), "application/json");
                } else {
                    res.status = 404;
                    res.set_content(json({{"success", false}, {"message", "Team not found."}}).dump(), "application/json");
                }
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // ========================================================
        // 5. Rescue Assignment Workflow (Module 8)
        // ========================================================
        // Preview Assignment Recommendation (NO state change)
        server.Get("/api/assignment/recommend", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!req.has_param("reportId")) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", "Query parameter 'reportId' is required."}}).dump(), "application/json");
                return;
            }

            std::string reportId = req.get_param_value("reportId");
            models::Request report;
            if (!rescueService.findReport(reportId, report)) {
                res.status = 404;
                res.set_content(json({{"success", false}, {"message", "Report not found."}}).dump(), "application/json");
                return;
            }

            // 1. Identify suitable available teams matching requiredCapability
            auto allTeams = resourceService.getRescueTeams();
            std::vector<models::RescueTeam> eligibleTeams;

            for (const auto& t : allTeams) {
                if (t.status == "Available") {
                    if (report.requiredCapability.empty() || t.type == report.requiredCapability) {
                        eligibleTeams.push_back(t);
                    }
                }
            }

            // 2. Check required resources in inventory
            services::StockCheckResult stockCheck = resourceService.checkStock(report.requiredResourceType, report.requiredResourceQty);

            if (eligibleTeams.empty()) {
                res.set_content(json({
                    {"success", true},
                    {"isViable", false},
                    {"report", report.toJson()},
                    {"recommendedTeam", nullptr},
                    {"message", "No available rescue teams found in service fleet."},
                    {"resourceCheck", {
                        {"available", stockCheck.available},
                        {"currentStock", stockCheck.currentStock},
                        {"requestedStock", stockCheck.requestedStock},
                        {"shortage", stockCheck.shortage},
                        {"unit", stockCheck.unit}
                    }}
                }).dump(), "application/json");
                return;
            }

            // 3. For each eligible team, calculate shortest Dijkstra path to report location
            models::RescueTeam bestTeam;
            structures::DijkstraResult bestRoute;
            double minTravelTime = 1e9;
            bool foundRoute = false;

            for (const auto& team : eligibleTeams) {
                structures::DijkstraResult route = networkService.runDijkstra(team.currentLocation, report.location);
                if (route.isReachable && route.estimatedTimeMinutes < minTravelTime) {
                    minTravelTime = route.estimatedTimeMinutes;
                    bestTeam = team;
                    bestRoute = route;
                    foundRoute = true;
                }
            }

            if (!foundRoute) {
                res.set_content(json({
                    {"success", true},
                    {"isViable", false},
                    {"report", report.toJson()},
                    {"recommendedTeam", eligibleTeams[0].toJson()},
                    {"message", "No reachable route found from available team stations to " + report.location + " (roads may be blocked)."},
                    {"resourceCheck", {
                        {"available", stockCheck.available},
                        {"currentStock", stockCheck.currentStock},
                        {"requestedStock", stockCheck.requestedStock},
                        {"shortage", stockCheck.shortage},
                        {"unit", stockCheck.unit}
                    }}
                }).dump(), "application/json");
                return;
            }

            bool isViable = foundRoute && stockCheck.available;

            res.set_content(json({
                {"success", true},
                {"isViable", isViable},
                {"report", report.toJson()},
                {"recommendedTeam", bestTeam.toJson()},
                {"route", {
                    {"path", bestRoute.path},
                    {"totalDistance", bestRoute.totalDistance},
                    {"estimatedTimeMinutes", bestRoute.estimatedTimeMinutes},
                    {"segments", bestRoute.segments},
                    {"isReachable", bestRoute.isReachable}
                }},
                {"resourceCheck", {
                    {"available", stockCheck.available},
                    {"currentStock", stockCheck.currentStock},
                    {"requestedStock", stockCheck.requestedStock},
                    {"shortage", stockCheck.shortage},
                    {"unit", stockCheck.unit}
                }},
                {"message", isViable ? "Viable rescue mission verified via Dijkstra route." : "Mission requires attention (resource shortage or route constraint)."}
            }).dump(), "application/json");
        });

        // Confirm Assignment (operator explicit confirmation)
        server.Post("/api/assignment/confirm", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string reportId = body.value("reportId", "");
                std::string teamId = body.value("teamId", "");

                if (reportId.empty() || teamId.empty()) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "reportId and teamId are required."}}).dump(), "application/json");
                    return;
                }

                models::Request report;
                if (!rescueService.findReport(reportId, report)) {
                    res.status = 404;
                    res.set_content(json({{"success", false}, {"message", "Report not found."}}).dump(), "application/json");
                    return;
                }

                models::RescueTeam team;
                if (!resourceService.findTeam(teamId, team)) {
                    res.status = 404;
                    res.set_content(json({{"success", false}, {"message", "Team not found."}}).dump(), "application/json");
                    return;
                }

                // Verify team availability (Anti-Double Booking)
                if (team.status == "Busy") {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "Team '" + team.name + "' is already BUSY on another assignment."}}).dump(), "application/json");
                    return;
                }

                // Calculate travel time via Dijkstra
                structures::DijkstraResult route = networkService.runDijkstra(team.currentLocation, report.location);
                if (!route.isReachable) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "Cannot assign: No open route to " + report.location + " (roads blocked)."}}).dump(), "application/json");
                    return;
                }

                // Allocate required resources from Linked List inventory
                if (!report.requiredResourceType.empty() && report.requiredResourceQty > 0) {
                    models::Allocation alc = resourceService.allocate(report.id, report.personName, report.location, report.requiredResourceType, report.requiredResourceQty, "Assigned to " + team.name);
                    services::DatabaseService::getInstance().insertAllocation(alc);
                    services::DatabaseService::getInstance().deductResource(report.requiredResourceType, report.requiredResourceQty);
                }

                // Mark team Busy
                resourceService.assignTeam(teamId, report.location, report.id, route.estimatedTimeMinutes);
                services::DatabaseService::getInstance().updateTeamStatus(teamId, "Busy", report.id, report.location, route.estimatedTimeMinutes);

                // Update report status to Assigned
                rescueService.updateReportStatus(report.id, "Assigned", teamId);
                services::DatabaseService::getInstance().updateReportStatus(report.id, "Assigned", teamId);

                // Record in rescue_operations table
                services::DatabaseService::getInstance().insertOperationRecord(report.id, teamId, "Assigned", "Route ETA: " + std::to_string(static_cast<int>(route.estimatedTimeMinutes)) + " min");

                auto op = historyService.recordOperation("Mission Assigned: Team " + team.name + " (" + teamId + ") dispatched to #" + report.id + " at " + report.location + " (ETA " + std::to_string(static_cast<int>(route.estimatedTimeMinutes)) + " min)", "Request", report.id);
                services::DatabaseService::getInstance().insertHistory(op);

                res.set_content(json({
                    {"success", true},
                    {"message", "Assignment confirmed successfully."},
                    {"reportId", report.id},
                    {"teamId", teamId},
                    {"etaMinutes", route.estimatedTimeMinutes},
                    {"route", {
                        {"path", route.path},
                        {"segments", route.segments}
                    }}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // Status Transition: In Progress -> Completed / Cancelled
        server.Post("/api/assignment/update-status", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string reportId = body.value("reportId", "");
                std::string newStatus = body.value("status", ""); // "In Progress", "Completed", "Cancelled"

                if (reportId.empty() || newStatus.empty()) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "reportId and status are required."}}).dump(), "application/json");
                    return;
                }

                models::Request report;
                if (!rescueService.findReport(reportId, report)) {
                    res.status = 404;
                    res.set_content(json({{"success", false}, {"message", "Report not found."}}).dump(), "application/json");
                    return;
                }

                if (newStatus == "Completed") {
                    // Update report status to Completed
                    rescueService.updateReportStatus(reportId, "Completed");
                    services::DatabaseService::getInstance().updateReportStatus(reportId, "Completed");
                    services::DatabaseService::getInstance().updateOperationRecordStatus(reportId, "Completed");

                    // Release assigned team to Available status at report location
                    if (!report.assignedTeamId.empty()) {
                        resourceService.releaseTeam(report.assignedTeamId, report.location);
                        services::DatabaseService::getInstance().updateTeamStatus(report.assignedTeamId, "Available", "", report.location);
                    }

                    // Return reusable equipment (e.g. boats)
                    if (report.requiredResourceType == "Rescue Boats") {
                        resourceService.returnResource("Rescue Boats", report.requiredResourceQty);
                        services::DatabaseService::getInstance().returnResource("Rescue Boats", report.requiredResourceQty);
                    }

                    auto op = historyService.recordOperation("Rescue Mission #" + reportId + " COMPLETED at " + report.location + ". Team " + report.assignedTeamId + " released to Available.", "Request", reportId);
                    services::DatabaseService::getInstance().insertHistory(op);
                } else if (newStatus == "Cancelled") {
                    rescueService.updateReportStatus(reportId, "Cancelled");
                    services::DatabaseService::getInstance().updateReportStatus(reportId, "Cancelled");
                    services::DatabaseService::getInstance().updateOperationRecordStatus(reportId, "Cancelled");

                    if (!report.assignedTeamId.empty()) {
                        resourceService.releaseTeam(report.assignedTeamId);
                        services::DatabaseService::getInstance().updateTeamStatus(report.assignedTeamId, "Available", "", "Relief Camp");
                    }

                    // Return allocated supplies
                    if (!report.requiredResourceType.empty() && report.requiredResourceQty > 0) {
                        resourceService.returnResource(report.requiredResourceType, report.requiredResourceQty);
                        services::DatabaseService::getInstance().returnResource(report.requiredResourceType, report.requiredResourceQty);
                    }

                    auto op = historyService.recordOperation("Mission #" + reportId + " CANCELLED. Responders and resources returned.", "Request", reportId);
                    services::DatabaseService::getInstance().insertHistory(op);
                } else {
                    rescueService.updateReportStatus(reportId, newStatus);
                    services::DatabaseService::getInstance().updateReportStatus(reportId, newStatus);
                    services::DatabaseService::getInstance().updateOperationRecordStatus(reportId, newStatus);
                    auto op = historyService.recordOperation("Report #" + reportId + " transitioned to status: " + newStatus, "Request", reportId);
                    services::DatabaseService::getInstance().insertHistory(op);
                }

                res.set_content(json({
                    {"success", true},
                    {"message", "Status updated to " + newStatus},
                    {"reportId", reportId},
                    {"status", newStatus}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // ========================================================
        // 6. Resources & Inventory Management (Linked List)
        // ========================================================
        server.Get("/api/resources", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            auto inv = resourceService.getInventory();
            json arr = json::array();
            for (const auto& r : inv) arr.push_back(r.toJson());

            res.set_content(json({
                {"success", true},
                {"count", arr.size()},
                {"totalUnits", resourceService.getTotalUnits()},
                {"lowStockCount", resourceService.getLowStockCount()},
                {"resources", arr}
            }).dump(), "application/json");
        });

        server.Post("/api/resources", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string name = body.value("name", "");
                std::string category = body.value("category", "General Supplies");
                int quantity = body.value("quantity", 0);
                int minThreshold = body.value("minThreshold", 20);
                std::string unit = body.value("unit", "units");

                if (name.empty() || quantity <= 0) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "Valid resource name and positive quantity required."}}).dump(), "application/json");
                    return;
                }

                models::Resource r = resourceService.addOrReplenish(name, category, quantity, minThreshold, unit);
                auto op = historyService.recordOperation("Stock replenished: " + r.name + " (+" + std::to_string(quantity) + " " + r.unit + ")", "Resource", r.id);

                // Persist to SQLite
                services::DatabaseService::getInstance().insertOrUpdateResource(r);
                services::DatabaseService::getInstance().insertHistory(op);

                res.set_content(json({{"success", true}, {"resource", r.toJson()}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        server.Get("/api/allocations", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            const auto& list = resourceService.getAllocations();
            json arr = json::array();
            for (const auto& a : list) arr.push_back(a.toJson());
            res.set_content(json({{"success", true}, {"allocations", arr}}).dump(), "application/json");
        });

        server.Post("/api/resources/allocate", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string requestId = body.value("requestId", "");
                std::string resourceName = body.value("resourceName", "");
                int quantity = body.value("quantity", 0);
                std::string notes = body.value("notes", "");

                models::Request targetReq;
                if (!rescueService.findReport(requestId, targetReq)) {
                    res.status = 404;
                    res.set_content(json({{"success", false}, {"message", "Report with ID '" + requestId + "' does not exist."}}).dump(), "application/json");
                    return;
                }

                models::Allocation alc = resourceService.allocate(requestId, targetReq.personName, targetReq.location, resourceName, quantity, notes);
                auto op = historyService.recordOperation("Allocated " + std::to_string(quantity) + " " + resourceName + " to " + targetReq.location, "Allocation", alc.id);

                // Persist to SQLite
                services::DatabaseService::getInstance().insertAllocation(alc);
                services::DatabaseService::getInstance().deductResource(resourceName, quantity);
                services::DatabaseService::getInstance().insertHistory(op);

                res.set_content(json({
                    {"success", true},
                    {"message", "Successfully allocated " + std::to_string(quantity) + " units to " + targetReq.personName},
                    {"allocation", alc.toJson()}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // ========================================================
        // 7. Locations & Roads Network Management (Adjacency List Graph)
        // ========================================================
        server.Get("/api/locations", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            const auto& locs = networkService.getLocations();
            json arr = json::array();
            for (const auto& l : locs) arr.push_back(l.toJson());
            res.set_content(json({{"success", true}, {"locations", arr}}).dump(), "application/json");
        });

        server.Post("/api/locations", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string name = body.value("name", "");
                std::string type = body.value("type", "Affected Area");
                double x = body.value("x", 400.0);
                double y = body.value("y", 250.0);

                models::Location loc = networkService.addLocation(name, type, x, y);
                auto op = historyService.recordOperation("Location registered: " + name + " (" + type + ")", "Location", loc.id);

                // Persist to SQLite
                services::DatabaseService::getInstance().insertLocation(loc);
                services::DatabaseService::getInstance().insertHistory(op);

                res.status = 201;
                res.set_content(json({{"success", true}, {"location", loc.toJson()}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        server.Get("/api/roads", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            const auto& rds = networkService.getRoads();
            json arr = json::array();
            for (const auto& r : rds) arr.push_back(r.toJson());
            res.set_content(json({{"success", true}, {"roads", arr}}).dump(), "application/json");
        });

        server.Post("/api/roads", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string from = body.value("from", "");
                std::string to = body.value("to", "");
                double distance = body.value("distance", 5.0);
                std::string status = body.value("status", "Open");

                models::Road rd = networkService.addRoad(from, to, distance, status);
                auto op = historyService.recordOperation("Road registered: " + from + " ↔ " + to + " (" + std::to_string(static_cast<int>(distance)) + " min)", "Road", rd.id);

                // Persist to SQLite
                services::DatabaseService::getInstance().insertRoad(rd);
                services::DatabaseService::getInstance().insertHistory(op);

                res.status = 201;
                res.set_content(json({{"success", true}, {"road", rd.toJson()}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // Toggle Road Blocked / Reopen
        server.Post("/api/roads/toggle-block", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            try {
                auto body = json::parse(req.body);
                std::string roadId = body.value("roadId", "");
                if (roadId.empty()) {
                    res.status = 400;
                    res.set_content(json({{"success", false}, {"message", "roadId is required."}}).dump(), "application/json");
                    return;
                }

                models::Road updatedRoad = networkService.toggleRoadBlocked(roadId);
                auto op = historyService.recordOperation("Road status updated: " + updatedRoad.from + " ↔ " + updatedRoad.to + " is now " + updatedRoad.status, "Road", roadId);

                // Persist to SQLite
                services::DatabaseService::getInstance().updateRoadStatus(roadId, updatedRoad.status);
                services::DatabaseService::getInstance().insertHistory(op);

                res.set_content(json({
                    {"success", true},
                    {"message", "Road status changed to " + updatedRoad.status},
                    {"road", updatedRoad.toJson()}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // ========================================================
        // 8. Rescue Route Exploration (Dijkstra, BFS, DFS)
        // ========================================================
        server.Get("/api/routes/dijkstra", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!req.has_param("start") || !req.has_param("destination")) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", "Both 'start' and 'destination' query parameters required."}}).dump(), "application/json");
                return;
            }
            std::string start = req.get_param_value("start");
            std::string dest = req.get_param_value("destination");

            try {
                structures::DijkstraResult result = networkService.runDijkstra(start, dest);
                historyService.recordOperation("Dijkstra Route calculated: " + start + " → " + dest + " (" +
                                               std::to_string(static_cast<int>(result.totalDistance * 10) / 10.0) + " min)", "Network");

                res.set_content(json({
                    {"success", true},
                    {"algorithm", "Dijkstra Shortest Path"},
                    {"result", {
                        {"startLocation", result.startLocation},
                        {"destinationLocation", result.destinationLocation},
                        {"path", result.path},
                        {"totalDistance", result.totalDistance},
                        {"estimatedTimeMinutes", result.estimatedTimeMinutes},
                        {"isReachable", result.isReachable},
                        {"segments", result.segments}
                    }}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        server.Get("/api/routes/bfs", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!req.has_param("start")) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", "Query parameter 'start' is required."}}).dump(), "application/json");
                return;
            }
            std::string start = req.get_param_value("start");
            try {
                auto result = networkService.runBFS(start);
                historyService.recordOperation("BFS Exploration executed from '" + start + "'", "Network");
                res.set_content(json({
                    {"success", true},
                    {"result", {
                        {"algorithm", result.algorithm},
                        {"startLocation", result.startLocation},
                        {"traversalOrder", result.traversalOrder},
                        {"unreachableLocations", result.unreachableLocations},
                        {"totalVertices", result.totalVertices},
                        {"visitedCount", result.visitedCount},
                        {"unreachableCount", result.unreachableCount}
                    }}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        server.Get("/api/routes/dfs", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!req.has_param("start")) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", "Query parameter 'start' is required."}}).dump(), "application/json");
                return;
            }
            std::string start = req.get_param_value("start");
            try {
                auto result = networkService.runDFS(start);
                historyService.recordOperation("DFS Exploration executed from '" + start + "'", "Network");
                res.set_content(json({
                    {"success", true},
                    {"result", {
                        {"algorithm", result.algorithm},
                        {"startLocation", result.startLocation},
                        {"traversalOrder", result.traversalOrder},
                        {"unreachableLocations", result.unreachableLocations},
                        {"totalVertices", result.totalVertices},
                        {"visitedCount", result.visitedCount},
                        {"unreachableCount", result.unreachableCount}
                    }}
                }).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"message", e.what()}}).dump(), "application/json");
            }
        });

        // ========================================================
        // 9. Operation History (Stack)
        // ========================================================
        server.Get("/api/history", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            std::string mode = req.has_param("mode") ? req.get_param_value("mode") : "recent";

            json ops = json::array();
            if (mode == "all") {
                const auto& full = historyService.getFullHistory();
                for (auto it = full.rbegin(); it != full.rend(); ++it) {
                    ops.push_back(it->toJson());
                }
            } else {
                auto recent = historyService.getRecentStackOperations();
                for (const auto& op : recent) ops.push_back(op.toJson());
            }

            res.set_content(json({
                {"success", true},
                {"source", (mode == "all" ? "Persistent History Log" : "C++ LIFO Stack")},
                {"count", ops.size()},
                {"operations", ops}
            }).dump(), "application/json");
        });

        // ========================================================
        // 10. Settings & Data Persistence
        // ========================================================
        server.Post("/api/save", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            bool ok = saveAllDataInternal();
            if (ok) {
                historyService.recordOperation("Application datasets saved to persistent storage files.", "System");
                res.set_content(json({
                    {"success", true},
                    {"message", "All records successfully persisted to data/ directory."},
                    {"savedFiles", {
                        "data/requests.txt",
                        "data/resources.txt",
                        "data/teams.txt",
                        "data/allocations.txt",
                        "data/locations.txt",
                        "data/roads.txt",
                        "data/history.txt"
                    }}
                }).dump(), "application/json");
            } else {
                res.status = 500;
                res.set_content(json({{"success", false}, {"message", "Failed to write data files."}}).dump(), "application/json");
            }
        });

        // Reset Demonstration Data (Section 8 standard dataset)
        server.Post("/api/demo", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            seedDemoDataInternal();
            saveAllDataInternal();

            res.set_content(json({
                {"success", true},
                {"message", "Section 8 demonstration dataset successfully initialized."},
                {"dataset", "Relief Camp, Village A-C, Medical Center | Teams T01-T03 | Reports R101-R103"}
            }).dump(), "application/json");
        });

        // Export All Application Records as JSON
        server.Get("/api/export", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(dataMutex);
            json exportData;

            // Reports
            json repArr = json::array();
            for (const auto& r : rescueService.getAllReports()) repArr.push_back(r.toJson());
            exportData["reports"] = repArr;

            // Teams
            json teamArr = json::array();
            for (const auto& t : resourceService.getRescueTeams()) teamArr.push_back(t.toJson());
            exportData["teams"] = teamArr;

            // Resources
            json resArr = json::array();
            for (const auto& r : resourceService.getInventory()) resArr.push_back(r.toJson());
            exportData["resources"] = resArr;

            // Locations
            json locArr = json::array();
            for (const auto& l : networkService.getLocations()) locArr.push_back(l.toJson());
            exportData["locations"] = locArr;

            // Roads
            json roadArr = json::array();
            for (const auto& rd : networkService.getRoads()) roadArr.push_back(rd.toJson());
            exportData["roads"] = roadArr;

            // History
            json histArr = json::array();
            for (const auto& h : historyService.getFullHistory()) histArr.push_back(h.toJson());
            exportData["history"] = histArr;

            res.set_content(exportData.dump(2), "application/json");
        });
    }

    void start() {
        std::cout << "\n============================================================\n";
        std::cout << "  DISASTER RESCUE & MANAGEMENT SYSTEM (C++17)\n";
        std::cout << "  Manual DSA: Queue, Stack, LinkedList, PriorityQueue, Graph\n";
        std::cout << "============================================================\n";
        std::cout << "  Local Server: http://localhost:" << port << "\n";
        std::cout << "  Web UI:       http://localhost:" << port << "/index.html\n";
        std::cout << "============================================================\n\n";

        if (!server.listen("127.0.0.1", port)) {
            std::cerr << "[ERROR] Failed to bind to 127.0.0.1:" << port << "\n";
        }
    }

    void stop() {
        server.stop();
    }
};

} // namespace api
