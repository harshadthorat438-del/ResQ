#ifndef LOCATIONS_H
#define LOCATIONS_H

#include <string>
#include <vector>
#include <utility>

constexpr int MAX_LOCATIONS = 20;

struct TraversalStep {
    std::string current;
    std::vector<std::string> visitedSoFar;
    std::vector<std::string> dsState; // Queue elements for BFS, Stack for DFS
    std::string fromNode;
    std::string explanation;
};

struct TraversalResult {
    std::string type; // "BFS" or "DFS"
    std::string startNode;
    std::vector<std::string> order;
    std::vector<TraversalStep> steps;
};

class LocationGraph {
private:
    std::string locations[MAX_LOCATIONS];
    int adj[MAX_LOCATIONS][MAX_LOCATIONS];
    int locationCount;

    void dfsRecursive(int u, bool visited[]) const;
    void dfsTraceRecursive(int u, int parent, bool visited[], std::vector<std::string>& callStack,
                           TraversalResult& result) const;

public:
    LocationGraph();

    bool addLocation(const std::string& name);
    int findLocation(const std::string& name) const;
    bool addRoad(const std::string& first, const std::string& second);
    
    void bfs(const std::string& startName) const;
    void dfs(const std::string& startName) const;
    void displayLocations() const;

    TraversalResult bfsWithTrace(const std::string& startName) const;
    TraversalResult dfsWithTrace(const std::string& startName) const;

    int getLocationCount() const;
    int getRoadCount() const;
    std::string getLocationName(int index) const;
    std::vector<std::string> getAllLocations() const;
    std::vector<std::pair<std::string, std::string>> getAllRoads() const;
    std::vector<std::vector<int>> getAdjacencyMatrix() const;
    void clearAll();

    void saveToFile(const std::string& locFile = "data/locations.txt",
                    const std::string& roadFile = "data/roads.txt") const;
    void loadFromFile(const std::string& locFile = "data/locations.txt",
                      const std::string& roadFile = "data/roads.txt");
};

#endif // LOCATIONS_H
