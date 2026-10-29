#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include "../models/Location.h"
#include "../models/Road.h"
#include "../structures/Graph.h"
#include "StorageService.h"

namespace services {

class NetworkService {
private:
    // Core DSA: Graph using Adjacency List
    structures::Graph graph;

    std::vector<models::Location> locations;
    std::vector<models::Road> roads;
    int locIdCounter{1};
    int roadIdCounter{1};

public:
    NetworkService() = default;

    /**
     * @brief Adds a new location vertex to the disaster graph.
     * @throws std::invalid_argument if location name is empty or already exists.
     */
    models::Location addLocation(const std::string& name,
                                 const std::string& type = "Village",
                                 double x = 0.0,
                                 double y = 0.0,
                                 const std::string& customId = "") {
        if (name.empty()) {
            throw std::invalid_argument("Location name cannot be empty.");
        }

        for (const auto& loc : locations) {
            if (loc.name == name) {
                throw std::invalid_argument("Location '" + name + "' is already registered in the network.");
            }
        }

        models::Location loc;
        if (!customId.empty()) {
            loc.id = customId;
        } else {
            loc.id = "LOC-" + std::to_string(locIdCounter++);
        }
        loc.name = name;
        loc.type = type.empty() ? "Village" : type;
        loc.x = x;
        loc.y = y;

        locations.push_back(loc);
        graph.addVertex(name);
        return loc;
    }

    /**
     * @brief Adds a bidirectional road edge between two registered locations.
     * @throws std::invalid_argument if endpoints are invalid, identical, or edge already exists.
     */
    models::Road addRoad(const std::string& from,
                         const std::string& to,
                         double distance = 1.0,
                         const std::string& status = "Open",
                         const std::string& customId = "") {
        if (from.empty() || to.empty()) {
            throw std::invalid_argument("Both road endpoints must be specified.");
        }
        if (from == to) {
            throw std::invalid_argument("Self-loop road is not allowed.");
        }

        bool hasFrom = false;
        bool hasTo = false;
        for (const auto& loc : locations) {
            if (loc.name == from) hasFrom = true;
            if (loc.name == to) hasTo = true;
        }

        if (!hasFrom || !hasTo) {
            throw std::invalid_argument("Both endpoints ('" + from + "' and '" + to + "') must exist in registered locations.");
        }

        for (const auto& r : roads) {
            if ((r.from == from && r.to == to) || (r.from == to && r.to == from)) {
                throw std::invalid_argument("A road connection between '" + from + "' and '" + to + "' already exists.");
            }
        }

        models::Road road;
        if (!customId.empty()) {
            road.id = customId;
        } else {
            road.id = "RD-" + std::to_string(roadIdCounter++);
        }
        road.from = from;
        road.to = to;
        road.distance = distance > 0 ? distance : 1.0;
        road.status = status.empty() ? "Open" : status;

        roads.push_back(road);
        graph.addEdge(from, to, road.id, road.distance, road.status);
        return road;
    }

    /**
     * @brief Toggles road blocked/unblocked status and updates the underlying graph edge.
     */
    models::Road toggleRoadBlocked(const std::string& roadId) {
        for (auto& r : roads) {
            if (r.id == roadId) {
                if (r.status == "Blocked") {
                    r.status = "Open";
                } else {
                    r.status = "Blocked";
                }
                graph.updateEdgeStatus(roadId, r.status);
                return r;
            }
        }
        throw std::runtime_error("Road with ID '" + roadId + "' not found.");
    }

    /**
     * @brief Updates road status and estimated travel time in both network record and graph edge.
     */
    models::Road updateRoad(const std::string& roadId, double newDistance, const std::string& newStatus) {
        for (auto& r : roads) {
            if (r.id == roadId) {
                if (newDistance > 0) r.distance = newDistance;
                if (!newStatus.empty()) r.status = newStatus;

                // Rebuild graph edge
                graph.updateEdgeStatus(roadId, r.status);
                return r;
            }
        }
        throw std::runtime_error("Road with ID '" + roadId + "' not found.");
    }

    /**
     * @brief Executes BFS exploration starting at startLocation using custom FIFO Queue.
     */
    structures::TraversalResult runBFS(const std::string& startLocation) const {
        if (!graph.hasVertex(startLocation)) {
            throw std::invalid_argument("Starting location '" + startLocation + "' does not exist in network.");
        }
        return graph.bfs(startLocation);
    }

    /**
     * @brief Executes DFS exploration starting at startLocation using custom LIFO Stack.
     */
    structures::TraversalResult runDFS(const std::string& startLocation) const {
        if (!graph.hasVertex(startLocation)) {
            throw std::invalid_argument("Starting location '" + startLocation + "' does not exist in network.");
        }
        return graph.dfs(startLocation);
    }

    /**
     * @brief Computes shortest weighted rescue path using Dijkstra's algorithm.
     * Excludes blocked roads entirely.
     */
    structures::DijkstraResult runDijkstra(const std::string& startLocation, const std::string& destLocation) const {
        if (!graph.hasVertex(startLocation)) {
            throw std::invalid_argument("Origin location '" + startLocation + "' does not exist in network.");
        }
        if (!graph.hasVertex(destLocation)) {
            throw std::invalid_argument("Destination location '" + destLocation + "' does not exist in network.");
        }
        return graph.dijkstra(startLocation, destLocation);
    }

    const std::vector<models::Location>& getLocations() const {
        return locations;
    }

    const std::vector<models::Road>& getRoads() const {
        return roads;
    }

    size_t getLocationCount() const {
        return locations.size();
    }

    size_t getRoadCount() const {
        return roads.size();
    }

    size_t getBlockedRoadCount() const {
        size_t count = 0;
        for (const auto& r : roads) {
            if (r.status == "Blocked") count++;
        }
        return count;
    }

    void clear() {
        graph.clear();
        locations.clear();
        roads.clear();
        locIdCounter = 1;
        roadIdCounter = 1;
    }

    /**
     * @brief Saves locations to data/locations.txt.
     */
    bool saveLocationsToFile(const std::string& filepath = "data/locations.txt") const {
        StorageService::ensureDirectory();
        std::ofstream out(filepath, std::ios::trunc);
        if (!out.is_open()) return false;

        for (const auto& loc : locations) {
            out << StorageService::sanitize(loc.id) << "|"
                << StorageService::sanitize(loc.name) << "|"
                << StorageService::sanitize(loc.type) << "|"
                << loc.x << "|"
                << loc.y << "\n";
        }
        return true;
    }

    /**
     * @brief Loads locations from data/locations.txt and reconstructs vertices in graph.
     */
    bool loadLocationsFromFile(const std::string& filepath = "data/locations.txt") {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        locations.clear();
        graph.clear();
        std::string line;
        int maxIdNum = 0;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto tokens = StorageService::split(line, '|');
            if (tokens.size() >= 5) {
                models::Location loc;
                loc.id = tokens[0];
                loc.name = tokens[1];
                loc.type = tokens[2];
                try {
                    loc.x = std::stod(tokens[3]);
                    loc.y = std::stod(tokens[4]);
                } catch (...) {
                    loc.x = 0.0;
                    loc.y = 0.0;
                }

                if (loc.id.rfind("LOC-", 0) == 0) {
                    try {
                        int num = std::stoi(loc.id.substr(4));
                        if (num > maxIdNum) maxIdNum = num;
                    } catch (...) {}
                }

                locations.push_back(loc);
                graph.addVertex(loc.name);
            }
        }
        locIdCounter = maxIdNum + 1;
        return true;
    }

    /**
     * @brief Saves roads to data/roads.txt.
     */
    bool saveRoadsToFile(const std::string& filepath = "data/roads.txt") const {
        StorageService::ensureDirectory();
        std::ofstream out(filepath, std::ios::trunc);
        if (!out.is_open()) return false;

        for (const auto& r : roads) {
            out << StorageService::sanitize(r.id) << "|"
                << StorageService::sanitize(r.from) << "|"
                << StorageService::sanitize(r.to) << "|"
                << r.distance << "|"
                << StorageService::sanitize(r.status) << "\n";
        }
        return true;
    }

    /**
     * @brief Loads roads from data/roads.txt and reconstructs edges in graph.
     */
    bool loadRoadsFromFile(const std::string& filepath = "data/roads.txt") {
        std::ifstream in(filepath);
        if (!in.is_open()) return false;

        roads.clear();
        std::string line;
        int maxIdNum = 0;

        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto tokens = StorageService::split(line, '|');
            if (tokens.size() >= 5) {
                models::Road r;
                r.id = tokens[0];
                r.from = tokens[1];
                r.to = tokens[2];
                try {
                    r.distance = std::stod(tokens[3]);
                } catch (...) {
                    r.distance = 1.0;
                }
                r.status = tokens[4];

                if (r.id.rfind("RD-", 0) == 0) {
                    try {
                        int num = std::stoi(r.id.substr(3));
                        if (num > maxIdNum) maxIdNum = num;
                    } catch (...) {}
                }

                roads.push_back(r);
                try {
                    graph.addEdge(r.from, r.to, r.id, r.distance, r.status);
                } catch (...) {}
            }
        }
        roadIdCounter = maxIdNum + 1;
        return true;
    }
};

} // namespace services
