#pragma once
/**
 * @file Graph.h
 * @brief Manual Implementation of an Adjacency List Graph for Disaster Network Exploration.
 * 
 * Fundamentals of Data Structures (FDS) Specification:
 * - Representation: Adjacency List.
 *   Rationale: Real-world disaster road networks are sparse (E << V^2).
 *   Adjacency List provides optimal space complexity of O(V + E) compared to O(V^2) for
 *   an adjacency matrix, and enables direct O(degree(v)) iteration over accessible neighbors.
 * - Graph Type: Undirected / Bidirectional road networks.
 * - Traversal Algorithms:
 *   1. BFS (Breadth-First Search): Level-by-level exploration using our custom FIFO Queue.
 *   2. DFS (Depth-First Search): Branch-depth exploration using our custom LIFO Stack.
 * 
 * Complexity Analysis:
 * - addVertex(v):    O(1) amortized
 * - addEdge(u, v):   O(degree(u) + degree(v)) to prevent duplicates
 * - hasEdge(u, v):   O(degree(u))
 * - BFS traversal:   O(V + E) time, O(V) auxiliary space (visited array + Queue)
 * - DFS traversal:   O(V + E) time, O(V) auxiliary space (visited array + Stack)
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include "Queue.h"
#include "Stack.h"

namespace structures {

struct Edge {
    int targetIndex;
    std::string roadId;
    double distance;
    std::string status; // "Open", "Hazardous", "Blocked"

    Edge(int target, std::string id = "", double dist = 1.0, std::string st = "Open")
        : targetIndex(target), roadId(std::move(id)), distance(dist), status(std::move(st)) {}
};

struct TraversalResult {
    std::string algorithm; // "BFS" or "DFS"
    std::string startLocation;
    std::vector<std::string> traversalOrder;
    std::vector<std::string> unreachableLocations;
    size_t totalVertices;
    size_t visitedCount;
    size_t unreachableCount;
};

struct DijkstraResult {
    std::string startLocation;
    std::string destinationLocation;
    std::vector<std::string> path;
    double totalDistance{0.0};
    double estimatedTimeMinutes{0.0};
    bool isReachable{false};
    std::vector<std::string> segments;
};

class Graph {
private:
    std::vector<std::string> vertexNames;
    std::unordered_map<std::string, int> nameToIndex;
    std::vector<std::vector<Edge>> adjList;

public:
    Graph() = default;

    /**
     * @brief Adds a new vertex (disaster location) to the graph.
     * @return int Index of the newly added vertex, or existing index if already present.
     */
    int addVertex(const std::string& name) {
        if (name.empty()) {
            throw std::invalid_argument("Vertex name cannot be empty.");
        }
        auto it = nameToIndex.find(name);
        if (it != nameToIndex.end()) {
            return it->second;
        }

        int index = static_cast<int>(vertexNames.size());
        vertexNames.push_back(name);
        nameToIndex[name] = index;
        adjList.emplace_back();
        return index;
    }

    /**
     * @brief Checks if a location vertex exists.
     */
    bool hasVertex(const std::string& name) const {
        return nameToIndex.find(name) != nameToIndex.end();
    }

    /**
     * @brief Retrieves vertex index by name, or -1 if not found.
     */
    int getVertexIndex(const std::string& name) const {
        auto it = nameToIndex.find(name);
        return (it != nameToIndex.end()) ? it->second : -1;
    }

    /**
     * @brief Retrieves vertex name by index.
     */
    std::string getVertexName(int index) const {
        if (index >= 0 && index < static_cast<int>(vertexNames.size())) {
            return vertexNames[index];
        }
        return "";
    }

    /**
     * @brief Adds a bidirectional edge between two vertices with duplicate checking.
     * @throws std::invalid_argument if either vertex does not exist or endpoints are identical.
     */
    bool addEdge(const std::string& uName, const std::string& vName,
                 const std::string& roadId = "", double distance = 1.0,
                 const std::string& status = "Open") {
        if (uName == vName) {
            throw std::invalid_argument("Self-loop road is not allowed.");
        }

        int u = getVertexIndex(uName);
        int v = getVertexIndex(vName);

        if (u == -1 || v == -1) {
            throw std::invalid_argument("Both road endpoints must be registered locations.");
        }

        // Prevent duplicate edge
        for (const auto& edge : adjList[u]) {
            if (edge.targetIndex == v) {
                return false; // Edge already exists
            }
        }

        // Add bidirectional edges
        adjList[u].emplace_back(v, roadId, distance, status);
        adjList[v].emplace_back(u, roadId, distance, status);
        return true;
    }

    /**
     * @brief Updates the operational status (Open, Blocked, Hazardous) of an edge by roadId.
     */
    bool updateEdgeStatus(const std::string& roadId, const std::string& newStatus) {
        bool updated = false;
        for (auto& neighbors : adjList) {
            for (auto& edge : neighbors) {
                if (edge.roadId == roadId) {
                    edge.status = newStatus;
                    updated = true;
                }
            }
        }
        return updated;
    }

    /**
     * @brief Checks whether an edge exists between two vertices.
     */
    bool hasEdge(const std::string& uName, const std::string& vName) const {
        int u = getVertexIndex(uName);
        int v = getVertexIndex(vName);
        if (u == -1 || v == -1) return false;

        for (const auto& edge : adjList[u]) {
            if (edge.targetIndex == v) return true;
        }
        return false;
    }

    /**
     * @brief Returns neighbor vertex names for a given vertex.
     */
    std::vector<std::string> getNeighbors(const std::string& name) const {
        std::vector<std::string> neighbors;
        int u = getVertexIndex(name);
        if (u == -1) return neighbors;

        for (const auto& edge : adjList[u]) {
            neighbors.push_back(vertexNames[edge.targetIndex]);
        }
        return neighbors;
    }

    /**
     * @brief Executes Breadth-First Search (BFS) using custom FIFO Queue.
     * @param startVertex Starting location name.
     * @return TraversalResult detailed report.
     * 
     * Time Complexity: O(V + E)
     * Space Complexity: O(V)
     */
    TraversalResult bfs(const std::string& startVertex) const {
        TraversalResult result;
        result.algorithm = "BFS";
        result.startLocation = startVertex;
        result.totalVertices = vertexNames.size();

        int startIndex = getVertexIndex(startVertex);
        if (startIndex == -1 || vertexNames.empty()) {
            return result;
        }

        std::vector<bool> visited(vertexNames.size(), false);
        structures::Queue<int> queue; // Custom FDS Queue

        visited[startIndex] = true;
        queue.enqueue(startIndex);

        while (!queue.isEmpty()) {
            int current = queue.dequeue();
            result.traversalOrder.push_back(vertexNames[current]);

            // Explore adjacent vertices (sorted for deterministic traversal)
            std::vector<int> adjIndices;
            for (const auto& edge : adjList[current]) {
                if (edge.status != "Blocked") {
                    adjIndices.push_back(edge.targetIndex);
                }
            }
            std::sort(adjIndices.begin(), adjIndices.end(), [this](int a, int b) {
                return vertexNames[a] < vertexNames[b];
            });

            for (int neighbor : adjIndices) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.enqueue(neighbor);
                }
            }
        }

        // Identify unreachable vertices
        for (size_t i = 0; i < vertexNames.size(); ++i) {
            if (!visited[i]) {
                result.unreachableLocations.push_back(vertexNames[i]);
            }
        }

        result.visitedCount = result.traversalOrder.size();
        result.unreachableCount = result.unreachableLocations.size();
        return result;
    }

    /**
     * @brief Executes Depth-First Search (DFS) using custom LIFO Stack.
     * @param startVertex Starting location name.
     * @return TraversalResult detailed report.
     * 
     * Time Complexity: O(V + E)
     * Space Complexity: O(V)
     */
    TraversalResult dfs(const std::string& startVertex) const {
        TraversalResult result;
        result.algorithm = "DFS";
        result.startLocation = startVertex;
        result.totalVertices = vertexNames.size();

        int startIndex = getVertexIndex(startVertex);
        if (startIndex == -1 || vertexNames.empty()) {
            return result;
        }

        std::vector<bool> visited(vertexNames.size(), false);
        structures::Stack<int> stack; // Custom FDS Stack

        stack.push(startIndex);

        while (!stack.isEmpty()) {
            int current = stack.pop();

            if (!visited[current]) {
                visited[current] = true;
                result.traversalOrder.push_back(vertexNames[current]);

                // Collect unvisited open neighbors in reverse-sorted order
                std::vector<int> unvisitedNeighbors;
                for (const auto& edge : adjList[current]) {
                    if (edge.status != "Blocked" && !visited[edge.targetIndex]) {
                        unvisitedNeighbors.push_back(edge.targetIndex);
                    }
                }
                std::sort(unvisitedNeighbors.begin(), unvisitedNeighbors.end(), [this](int a, int b) {
                    return vertexNames[a] > vertexNames[b];
                });

                for (int neighbor : unvisitedNeighbors) {
                    stack.push(neighbor);
                }
            }
        }

        // Identify unreachable vertices
        for (size_t i = 0; i < vertexNames.size(); ++i) {
            if (!visited[i]) {
                result.unreachableLocations.push_back(vertexNames[i]);
            }
        }

        result.visitedCount = result.traversalOrder.size();
        result.unreachableCount = result.unreachableLocations.size();
        return result;
    }

    /**
     * @brief Computes the shortest weighted rescue route using Dijkstra's Algorithm.
     * Excludes blocked roads completely as required by emergency navigation protocol.
     * @param startVertex Name of starting origin (e.g. Relief Camp).
     * @param destVertex Name of destination disaster zone (e.g. Village C).
     * @return DijkstraResult containing exact path, total travel time (min), and reachability.
     * 
     * Time Complexity: O((V + E) log V)
     * Space Complexity: O(V)
     */
    DijkstraResult dijkstra(const std::string& startVertex, const std::string& destVertex) const {
        DijkstraResult result;
        result.startLocation = startVertex;
        result.destinationLocation = destVertex;
        result.isReachable = false;

        int startIndex = getVertexIndex(startVertex);
        int destIndex = getVertexIndex(destVertex);

        if (startIndex == -1 || destIndex == -1 || vertexNames.empty()) {
            return result;
        }

        if (startIndex == destIndex) {
            result.path.push_back(startVertex);
            result.totalDistance = 0.0;
            result.estimatedTimeMinutes = 0.0;
            result.isReachable = true;
            return result;
        }

        const size_t n = vertexNames.size();
        const double INF = 1e9;
        std::vector<double> dist(n, INF);
        std::vector<int> prev(n, -1);
        std::vector<bool> visited(n, false);

        dist[startIndex] = 0.0;

        for (size_t iter = 0; iter < n; ++iter) {
            int u = -1;
            double minDist = INF;

            for (size_t i = 0; i < n; ++i) {
                if (!visited[i] && dist[i] < minDist) {
                    minDist = dist[i];
                    u = static_cast<int>(i);
                }
            }

            if (u == -1 || dist[u] >= INF) break;
            if (u == destIndex) break; // Reached target destination

            visited[u] = true;

            for (const auto& edge : adjList[u]) {
                int v = edge.targetIndex;
                if (visited[v]) continue;

                // STRICT REQUIREMENT: Blocked roads are impassable - skip entirely!
                if (edge.status == "Blocked") {
                    continue;
                }

                double weight = edge.distance;
                if (edge.status == "Hazardous") {
                    weight *= 1.4; // 40% penalty for hazardous roads
                }

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                }
            }
        }

        // Reconstruct path
        if (dist[destIndex] >= INF || prev[destIndex] == -1) {
            return result; // Target zone is unreachable
        }

        std::vector<int> pathIndices;
        int curr = destIndex;
        while (curr != -1) {
            pathIndices.push_back(curr);
            curr = prev[curr];
        }
        std::reverse(pathIndices.begin(), pathIndices.end());

        double totalTime = 0.0;
        for (size_t i = 0; i < pathIndices.size(); ++i) {
            result.path.push_back(vertexNames[pathIndices[i]]);

            if (i + 1 < pathIndices.size()) {
                int u = pathIndices[i];
                int v = pathIndices[i + 1];
                for (const auto& edge : adjList[u]) {
                    if (edge.targetIndex == v && edge.status != "Blocked") {
                        totalTime += edge.distance;
                        result.segments.push_back(vertexNames[u] + " <-> " + vertexNames[v] + " (" +
                                                  std::to_string(static_cast<int>(edge.distance * 10) / 10.0) + " min)");
                        break;
                    }
                }
            }
        }

        result.totalDistance = totalTime;
        result.estimatedTimeMinutes = totalTime;
        result.isReachable = true;
        return result;
    }

    /**
     * @brief Returns total number of vertices.
     */
    size_t getVertexCount() const {
        return vertexNames.size();
    }

    /**
     * @brief Returns total number of unique undirected edges.
     */
    size_t getEdgeCount() const {
        size_t total = 0;
        for (const auto& edges : adjList) {
            total += edges.size();
        }
        return total / 2; // Each undirected edge is stored twice
    }

    /**
     * @brief Returns copy of all vertex names.
     */
    const std::vector<std::string>& getVertices() const {
        return vertexNames;
    }

    /**
     * @brief Resets the graph.
     */
    void clear() {
        vertexNames.clear();
        nameToIndex.clear();
        adjList.clear();
    }
};

} // namespace structures
