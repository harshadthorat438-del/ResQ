#include "locations.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <queue>

LocationGraph::LocationGraph() : locationCount(0) {
    std::memset(adj, 0, sizeof(adj));
}

void LocationGraph::clearAll() {
    locationCount = 0;
    std::memset(adj, 0, sizeof(adj));
}

int LocationGraph::findLocation(const std::string& name) const {
    for (int i = 0; i < locationCount; ++i) {
        if (locations[i] == name) {
            return i;
        }
    }
    return -1;
}

bool LocationGraph::addLocation(const std::string& name) {
    if (name.empty()) {
        std::cout << "Location name cannot be empty.\n";
        return false;
    }

    if (locationCount >= MAX_LOCATIONS) {
        std::cout << "Error: Maximum location capacity (" << MAX_LOCATIONS << ") reached.\n";
        return false;
    }

    if (findLocation(name) != -1) {
        std::cout << "Location \"" << name << "\" already exists.\n";
        return false;
    }

    locations[locationCount++] = name;
    saveToFile();
    return true;
}

bool LocationGraph::addRoad(const std::string& first, const std::string& second) {
    int u = findLocation(first);
    int v = findLocation(second);

    if (u < 0 || v < 0 || u == v) {
        std::cout << "Invalid locations. Make sure both locations exist and are distinct.\n";
        return false;
    }

    if (adj[u][v] == 1) {
        std::cout << "Road between \"" << first << "\" and \"" << second << "\" already exists.\n";
        return false;
    }

    adj[u][v] = 1;
    adj[v][u] = 1;
    saveToFile();
    return true;
}

void LocationGraph::bfs(const std::string& startName) const {
    int start = findLocation(startName);
    if (start < 0) {
        std::cout << "Starting location \"" << startName << "\" not found.\n";
        return;
    }

    bool visited[MAX_LOCATIONS] = {false};
    int queue[MAX_LOCATIONS];
    int head = 0, tail = 0;

    queue[tail++] = start;
    visited[start] = true;

    std::cout << "BFS: ";
    while (head < tail) {
        int curr = queue[head++];
        std::cout << locations[curr] << " -> ";

        for (int i = 0; i < locationCount; ++i) {
            if (adj[curr][i] && !visited[i]) {
                visited[i] = true;
                queue[tail++] = i;
            }
        }
    }
    std::cout << "END\n";
}

void LocationGraph::dfsRecursive(int u, bool visited[]) const {
    visited[u] = true;
    std::cout << locations[u] << " -> ";

    for (int i = 0; i < locationCount; ++i) {
        if (adj[u][i] && !visited[i]) {
            dfsRecursive(i, visited);
        }
    }
}

void LocationGraph::dfs(const std::string& startName) const {
    int start = findLocation(startName);
    if (start < 0) {
        std::cout << "Starting location \"" << startName << "\" not found.\n";
        return;
    }

    bool visited[MAX_LOCATIONS] = {false};
    std::cout << "DFS: ";
    dfsRecursive(start, visited);
    std::cout << "END\n";
}

TraversalResult LocationGraph::bfsWithTrace(const std::string& startName) const {
    TraversalResult result;
    result.type = "BFS";
    result.startNode = startName;

    int start = findLocation(startName);
    if (start < 0) return result;

    bool visited[MAX_LOCATIONS] = {false};
    std::vector<int> q;
    q.push_back(start);
    visited[start] = true;
    result.order.push_back(locations[start]);

    std::vector<std::string> visitedNames = {locations[start]};
    std::vector<std::string> qNames = {locations[start]};

    TraversalStep initialStep;
    initialStep.current = locations[start];
    initialStep.fromNode = "";
    initialStep.visitedSoFar = visitedNames;
    initialStep.dsState = qNames;
    initialStep.explanation = "Starting at " + locations[start] + ". Enqueued into FIFO Queue and marked as visited.";
    result.steps.push_back(initialStep);

    size_t head = 0;
    while (head < q.size()) {
        int curr = q[head++];

        // Update queue display state
        qNames.clear();
        for (size_t k = head; k < q.size(); ++k) {
            qNames.push_back(locations[q[k]]);
        }

        for (int i = 0; i < locationCount; ++i) {
            if (adj[curr][i] && !visited[i]) {
                visited[i] = true;
                q.push_back(i);
                qNames.push_back(locations[i]);
                result.order.push_back(locations[i]);
                visitedNames.push_back(locations[i]);

                TraversalStep step;
                step.current = locations[i];
                step.fromNode = locations[curr];
                step.visitedSoFar = visitedNames;
                step.dsState = qNames;
                step.explanation = "From " + locations[curr] + ", discovered neighbor " + locations[i] + ". Enqueued into FIFO queue.";
                result.steps.push_back(step);
            }
        }
    }

    return result;
}

void LocationGraph::dfsTraceRecursive(int u, int parent, bool visited[], std::vector<std::string>& callStack,
                                     TraversalResult& result) const {
    visited[u] = true;
    callStack.push_back(locations[u]);
    result.order.push_back(locations[u]);

    std::vector<std::string> visitedNames;
    for (int i = 0; i < locationCount; ++i) {
        if (visited[i]) visitedNames.push_back(locations[i]);
    }

    TraversalStep step;
    step.current = locations[u];
    step.fromNode = (parent >= 0) ? locations[parent] : "";
    step.visitedSoFar = visitedNames;
    step.dsState = callStack;
    if (parent >= 0) {
        step.explanation = "Traversed road from " + locations[parent] + " to " + locations[u] + ". Pushed onto Call Stack (Depth " + std::to_string(callStack.size()) + ").";
    } else {
        step.explanation = "Starting DFS recursion at root " + locations[u] + ". Pushed onto Call Stack.";
    }
    result.steps.push_back(step);

    for (int i = 0; i < locationCount; ++i) {
        if (adj[u][i] && !visited[i]) {
            dfsTraceRecursive(i, u, visited, callStack, result);
        }
    }

    callStack.pop_back();
}

TraversalResult LocationGraph::dfsWithTrace(const std::string& startName) const {
    TraversalResult result;
    result.type = "DFS";
    result.startNode = startName;

    int start = findLocation(startName);
    if (start < 0) return result;

    bool visited[MAX_LOCATIONS] = {false};
    std::vector<std::string> callStack;
    dfsTraceRecursive(start, -1, visited, callStack, result);
    return result;
}

void LocationGraph::displayLocations() const {
    if (locationCount == 0) {
        std::cout << "No locations registered.\n";
        return;
    }

    std::cout << "\n--- REGISTERED LOCATIONS & CONNECTIONS ---\n";
    for (int i = 0; i < locationCount; ++i) {
        std::cout << "[" << i << "] " << locations[i] << " (Connected to: ";
        bool first = true;
        for (int j = 0; j < locationCount; ++j) {
            if (adj[i][j]) {
                if (!first) std::cout << ", ";
                std::cout << locations[j];
                first = false;
            }
        }
        if (first) std::cout << "None";
        std::cout << ")\n";
    }
}

int LocationGraph::getLocationCount() const {
    return locationCount;
}

int LocationGraph::getRoadCount() const {
    int roads = 0;
    for (int i = 0; i < locationCount; ++i) {
        for (int j = i + 1; j < locationCount; ++j) {
            if (adj[i][j]) roads++;
        }
    }
    return roads;
}

std::string LocationGraph::getLocationName(int index) const {
    if (index >= 0 && index < locationCount) {
        return locations[index];
    }
    return "";
}

std::vector<std::string> LocationGraph::getAllLocations() const {
    std::vector<std::string> list;
    for (int i = 0; i < locationCount; ++i) {
        list.push_back(locations[i]);
    }
    return list;
}

std::vector<std::pair<std::string, std::string>> LocationGraph::getAllRoads() const {
    std::vector<std::pair<std::string, std::string>> roads;
    for (int i = 0; i < locationCount; ++i) {
        for (int j = i + 1; j < locationCount; ++j) {
            if (adj[i][j]) {
                roads.push_back({locations[i], locations[j]});
            }
        }
    }
    return roads;
}

std::vector<std::vector<int>> LocationGraph::getAdjacencyMatrix() const {
    std::vector<std::vector<int>> mat(locationCount, std::vector<int>(locationCount, 0));
    for (int i = 0; i < locationCount; ++i) {
        for (int j = 0; j < locationCount; ++j) {
            mat[i][j] = adj[i][j];
        }
    }
    return mat;
}

void LocationGraph::saveToFile(const std::string& locFile, const std::string& roadFile) const {
    std::ofstream outLoc(locFile);
    if (outLoc.is_open()) {
        outLoc << locationCount << "\n";
        for (int i = 0; i < locationCount; ++i) {
            outLoc << locations[i] << "\n";
        }
    }

    std::ofstream outRoad(roadFile);
    if (outRoad.is_open()) {
        for (int i = 0; i < locationCount; ++i) {
            for (int j = i + 1; j < locationCount; ++j) {
                if (adj[i][j]) {
                    outRoad << i << " " << j << "\n";
                }
            }
        }
    }
}

void LocationGraph::loadFromFile(const std::string& locFile, const std::string& roadFile) {
    locationCount = 0;
    std::memset(adj, 0, sizeof(adj));

    std::ifstream inLoc(locFile);
    if (inLoc.is_open()) {
        int count = 0;
        if (inLoc >> count) {
            std::string dummy;
            std::getline(inLoc, dummy);
            if (count > MAX_LOCATIONS) count = MAX_LOCATIONS;

            for (int i = 0; i < count; ++i) {
                std::string name;
                if (std::getline(inLoc, name)) {
                    if (!name.empty()) {
                        locations[locationCount++] = name;
                    }
                }
            }
        }
    }

    std::ifstream inRoad(roadFile);
    if (inRoad.is_open()) {
        int u = 0, v = 0;
        while (inRoad >> u >> v) {
            if (u >= 0 && v >= 0 && u < locationCount && v < locationCount && u != v) {
                adj[u][v] = 1;
                adj[v][u] = 1;
            }
        }
    }
}
