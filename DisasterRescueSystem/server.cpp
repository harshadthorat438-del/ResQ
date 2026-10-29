#include <iostream>
#include <string>
#include <filesystem>
#include <mutex>
#include "httplib.h"
#include "json.hpp"
#include "rescue_queue.h"
#include "resources.h"
#include "locations.h"
#include "history.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

static std::mutex g_dataMutex;
static RescueQueue g_rescueQueue;
static ResourceManager g_resourceManager;
static LocationGraph g_locationGraph;
static OperationHistory g_history;

void ensureDataDirectory() {
    try {
        if (!fs::exists("data")) {
            fs::create_directories("data");
        }
    } catch (...) {}
}

void loadAllData() {
    std::lock_guard<std::mutex> lock(g_dataMutex);
    ensureDataDirectory();
    g_rescueQueue.loadFromFile();
    g_resourceManager.loadFromFile();
    g_locationGraph.loadFromFile();
    g_history.loadFromFile();
}

void seedDemoData() {
    std::lock_guard<std::mutex> lock(g_dataMutex);
    ensureDataDirectory();

    g_rescueQueue.clearAll();
    g_resourceManager.clearAll();
    g_locationGraph.clearAll();
    g_history.clearAll();

    // Seed Locations
    g_locationGraph.addLocation("Central Relief Hub");
    g_locationGraph.addLocation("North Sector Hospital");
    g_locationGraph.addLocation("Riverbank Village");
    g_locationGraph.addLocation("Hillside Shelter");
    g_locationGraph.addLocation("Bridge Checkpoint");
    g_locationGraph.addLocation("Valley Camp");

    // Seed Roads
    g_locationGraph.addRoad("Central Relief Hub", "North Sector Hospital");
    g_locationGraph.addRoad("Central Relief Hub", "Riverbank Village");
    g_locationGraph.addRoad("Central Relief Hub", "Bridge Checkpoint");
    g_locationGraph.addRoad("North Sector Hospital", "Hillside Shelter");
    g_locationGraph.addRoad("Riverbank Village", "Valley Camp");
    g_locationGraph.addRoad("Bridge Checkpoint", "Valley Camp");

    // Seed Resources
    g_resourceManager.addOrReplenish("Emergency Food Rations", 350);
    g_resourceManager.addOrReplenish("Sterile Medical Kits", 120);
    g_resourceManager.addOrReplenish("Potable Water Bottles (L)", 500);
    g_resourceManager.addOrReplenish("Thermal Blankets", 200);
    g_resourceManager.addOrReplenish("Inflatable Life Rafts", 25);

    // Seed Rescue Requests
    g_rescueQueue.enqueue("Riverbank Evacuation Group", "Riverbank Village", "Flash Flood Rising", 18);
    g_rescueQueue.enqueue("Senior Care Facility", "Hillside Shelter", "Power Failure & Medical Need", 9);
    g_rescueQueue.enqueue("Bridge Stranded Commuters", "Bridge Checkpoint", "Road Washout", 14);

    // Seed Operations
    g_history.push("System Initialized with Standard Disaster Network");
    g_history.push("Roads established connecting Central Relief Hub to surrounding sectors");
    g_history.push("Emergency supplies stocked into Central Inventory");
    g_history.push("Prioritized flash flood rescue request registered");
}

int main() {
    ensureDataDirectory();
    loadAllData();

    // If fresh install with empty state, populate initial demo scenario
    if (g_locationGraph.getLocationCount() == 0 && g_resourceManager.getCount() == 0 && g_rescueQueue.isEmpty()) {
        std::cout << "[INFO] Empty database detected. Loading initial disaster scenario...\n";
        seedDemoData();
    }

    httplib::Server svr;

    // Set CORS headers
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
    });

    // Mount static files directory
    if (!fs::exists("public")) {
        fs::create_directories("public");
    }
    svr.set_mount_point("/", "./public");

    // 1. Dashboard Status Overview
    svr.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);

        int totalDistressPeople = 0;
        auto requests = g_rescueQueue.getAllRequests();
        for (const auto& r : requests) {
            totalDistressPeople += r.people;
        }

        json j = {
            {"requestsCount", g_rescueQueue.getCount()},
            {"totalPeopleInDistress", totalDistressPeople},
            {"resourcesCount", g_resourceManager.getCount()},
            {"totalResourceUnits", g_resourceManager.getTotalQuantity()},
            {"locationsCount", g_locationGraph.getLocationCount()},
            {"roadsCount", g_locationGraph.getRoadCount()},
            {"historyCount", g_history.size()}
        };

        res.set_content(j.dump(), "application/json");
    });

    // 2. Rescue Queue API (Linked Queue, FIFO)
    svr.Get("/api/requests", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        auto list = g_rescueQueue.getAllRequests();

        json arr = json::array();
        for (const auto& r : list) {
            arr.push_back({
                {"id", r.id},
                {"name", r.name},
                {"location", r.location},
                {"emergency", r.emergency},
                {"people", r.people}
            });
        }

        json resp = {
            {"count", g_rescueQueue.getCount()},
            {"requests", arr}
        };
        res.set_content(resp.dump(), "application/json");
    });

    svr.Post("/api/requests", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        try {
            auto body = json::parse(req.body);
            std::string name = body.value("name", "");
            std::string location = body.value("location", "");
            std::string emergency = body.value("emergency", "");
            int people = body.value("people", 0);

            if (name.empty() || location.empty() || people <= 0) {
                res.status = 400;
                res.set_content(R"({"success":false,"message":"Invalid request fields"})", "application/json");
                return;
            }

            int id = g_rescueQueue.enqueue(name, location, emergency, people);
            if (id > 0) {
                std::string logMsg = "Request #" + std::to_string(id) + " enqueued: " + name + " at " + location;
                g_history.push(logMsg);

                json resp = {
                    {"success", true},
                    {"id", id},
                    {"message", "Rescue request #" + std::to_string(id) + " enqueued successfully."}
                };
                res.set_content(resp.dump(), "application/json");
            } else {
                res.status = 500;
                res.set_content(R"({"success":false,"message":"Failed to enqueue request"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(std::string("{\"success\":false,\"message\":\"JSON parse error: ") + e.what() + "\"}", "application/json");
        }
    });

    svr.Post("/api/requests/process", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        RescueRequest req(0, "", "", "", 0);
        if (g_rescueQueue.dequeue(req)) {
            std::string logMsg = "Processed request #" + std::to_string(req.id) + " (" + req.name + ")";
            g_history.push(logMsg);

            json resp = {
                {"success", true},
                {"message", "Processed rescue request #" + std::to_string(req.id)},
                {"processed", {
                    {"id", req.id},
                    {"name", req.name},
                    {"location", req.location},
                    {"emergency", req.emergency},
                    {"people", req.people}
                }}
            };
            res.set_content(resp.dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"success":false,"message":"Queue is empty. No pending rescue requests."})", "application/json");
        }
    });

    // 3. Resource Inventory API (Singly Linked List)
    svr.Get("/api/resources", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        auto list = g_resourceManager.getAllResources();

        json arr = json::array();
        for (const auto& item : list) {
            arr.push_back({
                {"name", item.first},
                {"quantity", item.second}
            });
        }

        json resp = {
            {"count", g_resourceManager.getCount()},
            {"totalUnits", g_resourceManager.getTotalQuantity()},
            {"resources", arr}
        };
        res.set_content(resp.dump(), "application/json");
    });

    svr.Post("/api/resources", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        try {
            auto body = json::parse(req.body);
            std::string name = body.value("name", "");
            int quantity = body.value("quantity", 0);

            if (name.empty() || quantity <= 0) {
                res.status = 400;
                res.set_content(R"({"success":false,"message":"Invalid resource name or non-positive quantity"})", "application/json");
                return;
            }

            if (g_resourceManager.addOrReplenish(name, quantity)) {
                std::string logMsg = "Stock updated: " + name + " (+" + std::to_string(quantity) + ")";
                g_history.push(logMsg);

                json resp = {
                    {"success", true},
                    {"message", "Resource \"" + name + "\" updated successfully."}
                };
                res.set_content(resp.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(R"({"success":false,"message":"Failed to update resource"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(std::string("{\"success\":false,\"message\":\"JSON error: ") + e.what() + "\"}", "application/json");
        }
    });

    svr.Post("/api/resources/allocate", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        try {
            auto body = json::parse(req.body);
            std::string name = body.value("name", "");
            int quantity = body.value("quantity", 0);

            int remaining = 0;
            if (g_resourceManager.allocate(name, quantity, remaining)) {
                std::string logMsg = "Allocated " + std::to_string(quantity) + " units of " + name;
                g_history.push(logMsg);

                json resp = {
                    {"success", true},
                    {"remaining", remaining},
                    {"message", "Allocated " + std::to_string(quantity) + " units. Remaining: " + std::to_string(remaining)}
                };
                res.set_content(resp.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(R"({"success":false,"message":"Resource not found or insufficient stock"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(std::string("{\"success\":false,\"message\":\"JSON error: ") + e.what() + "\"}", "application/json");
        }
    });

    // 4. Locations & Roads API (Graph with Adjacency Matrix)
    svr.Get("/api/locations", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        auto locs = g_locationGraph.getAllLocations();
        auto roads = g_locationGraph.getAllRoads();
        auto matrix = g_locationGraph.getAdjacencyMatrix();

        json roadsArr = json::array();
        for (const auto& rd : roads) {
            roadsArr.push_back({rd.first, rd.second});
        }

        json resp = {
            {"count", g_locationGraph.getLocationCount()},
            {"roadCount", g_locationGraph.getRoadCount()},
            {"locations", locs},
            {"roads", roadsArr},
            {"matrix", matrix}
        };
        res.set_content(resp.dump(), "application/json");
    });

    svr.Post("/api/locations", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        try {
            auto body = json::parse(req.body);
            std::string name = body.value("name", "");

            if (name.empty()) {
                res.status = 400;
                res.set_content(R"({"success":false,"message":"Location name cannot be empty"})", "application/json");
                return;
            }

            if (g_locationGraph.addLocation(name)) {
                std::string logMsg = "Location registered: " + name;
                g_history.push(logMsg);

                json resp = {{"success", true}, {"message", "Location \"" + name + "\" added successfully."}};
                res.set_content(resp.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(R"({"success":false,"message":"Location limit reached or duplicate name"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(std::string("{\"success\":false,\"message\":\"JSON error: ") + e.what() + "\"}", "application/json");
        }
    });

    svr.Post("/api/roads", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        try {
            auto body = json::parse(req.body);
            std::string first = body.value("first", "");
            std::string second = body.value("second", "");

            if (g_locationGraph.addRoad(first, second)) {
                std::string logMsg = "Road connected: " + first + " <-> " + second;
                g_history.push(logMsg);

                json resp = {{"success", true}, {"message", "Road connection established."}};
                res.set_content(resp.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(R"({"success":false,"message":"Invalid locations or road already connected"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(std::string("{\"success\":false,\"message\":\"JSON error: ") + e.what() + "\"}", "application/json");
        }
    });

    // 5. Graph Traversal API (BFS & DFS with live step-by-step trace)
    svr.Get("/api/traversal", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        std::string start = req.get_param_value("start");
        std::string type = req.get_param_value("type");

        if (start.empty()) {
            res.status = 400;
            res.set_content(R"({"success":false,"message":"Missing start location"})", "application/json");
            return;
        }

        TraversalResult result;
        if (type == "dfs" || type == "DFS") {
            result = g_locationGraph.dfsWithTrace(start);
        } else {
            result = g_locationGraph.bfsWithTrace(start);
        }

        json stepsArr = json::array();
        for (const auto& step : result.steps) {
            stepsArr.push_back({
                {"current", step.current},
                {"fromNode", step.fromNode},
                {"visitedSoFar", step.visitedSoFar},
                {"dsState", step.dsState},
                {"explanation", step.explanation}
            });
        }

        json resp = {
            {"type", result.type},
            {"startNode", result.startNode},
            {"order", result.order},
            {"steps", stepsArr}
        };

        std::string logMsg = "Executed " + result.type + " traversal originating from " + start;
        g_history.push(logMsg);

        res.set_content(resp.dump(), "application/json");
    });

    // 6. Operation History API (Array-based Stack, LIFO)
    svr.Get("/api/history", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        auto hist = g_history.getAllHistory();

        json resp = {
            {"count", g_history.size()},
            {"capacity", g_history.getCapacity()},
            {"history", hist}
        };
        res.set_content(resp.dump(), "application/json");
    });

    // 7. Reset / Demo Seed API
    svr.Post("/api/reset-demo", [](const httplib::Request&, httplib::Response& res) {
        seedDemoData();
        json resp = {
            {"success", true},
            {"message", "Disaster scenario demonstration data loaded successfully."}
        };
        res.set_content(resp.dump(), "application/json");
    });

    int port = 8080;
    std::cout << "\n======================================================\n";
    std::cout << "  DISASTER RESCUE & RESOURCE SYSTEM - WEB SERVER\n";
    std::cout << "======================================================\n";
    std::cout << " [OK] Backend data structures initialized (C++17)\n";
    std::cout << " [OK] REST API & Static file server listening on port " << port << "\n";
    std::cout << " Open your browser at: http://localhost:" << port << "\n";
    std::cout << " Press Ctrl+C in this console to stop the server.\n";
    std::cout << "======================================================\n\n";

    svr.listen("0.0.0.0", port);
    return 0;
}
