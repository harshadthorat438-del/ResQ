/**
 * Disaster Rescue & Resource Management System (Single File Version)
 * Language: C++ (C++17)
 * Implements: Linked Queue, Singly Linked List, Adjacency Matrix Graph (BFS & DFS), Array-based Stack
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <cstring>
#include <climits>
#include <filesystem>

namespace fs = std::filesystem;

constexpr int MAX_LOCATIONS = 20;
constexpr int MAX_HISTORY = 100;

/* =========================================================================
 * 1. STACK: OPERATION HISTORY (Array-based Stack, LIFO)
 * ========================================================================= */
class OperationHistory {
private:
    std::string history[MAX_HISTORY];
    int historyTop;

public:
    OperationHistory() : historyTop(-1) {}

    void push(const std::string& message) {
        if (message.empty()) return;
        if (historyTop == MAX_HISTORY - 1) {
            for (int i = 1; i < MAX_HISTORY; ++i) {
                history[i - 1] = history[i];
            }
            historyTop--;
        }
        history[++historyTop] = message;
        saveToFile();
    }

    bool pop(std::string& outMessage) {
        if (isEmpty()) return false;
        outMessage = history[historyTop--];
        saveToFile();
        return true;
    }

    void display() const {
        if (isEmpty()) {
            std::cout << "\nNo operations recorded yet.\n";
            return;
        }
        std::cout << "\n--- RECENT OPERATIONS (LIFO / Most Recent First) ---\n";
        for (int i = historyTop; i >= 0; --i) {
            std::cout << "[" << (historyTop - i + 1) << "] " << history[i] << "\n";
        }
    }

    bool isEmpty() const { return historyTop == -1; }

    void saveToFile(const std::string& filename = "data/history.txt") const {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) return;
        for (int i = 0; i <= historyTop; ++i) {
            outFile << history[i] << "\n";
        }
    }

    void loadFromFile(const std::string& filename = "data/history.txt") {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) return;
        historyTop = -1;
        std::string line;
        while (historyTop < MAX_HISTORY - 1 && std::getline(inFile, line)) {
            if (!line.empty()) {
                history[++historyTop] = line;
            }
        }
    }
};

/* =========================================================================
 * 2. LINKED LIST: RESOURCE INVENTORY (Singly Linked List)
 * ========================================================================= */
struct ResourceNode {
    std::string name;
    int quantity;
    ResourceNode* next;

    ResourceNode(const std::string& resName, int qty, ResourceNode* nextNode = nullptr)
        : name(resName), quantity(qty), next(nextNode) {}
};

class ResourceManager {
private:
    ResourceNode* head;

    void clear() {
        while (head) {
            ResourceNode* temp = head;
            head = head->next;
            delete temp;
        }
    }

public:
    ResourceManager() : head(nullptr) {}
    ~ResourceManager() { clear(); }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    ResourceNode* find(const std::string& name) const {
        ResourceNode* curr = head;
        while (curr) {
            if (curr->name == name) return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    bool addOrReplenish(const std::string& name, int quantity) {
        if (name.empty() || quantity <= 0) return false;
        ResourceNode* existing = find(name);
        if (existing) {
            if (INT_MAX - existing->quantity < quantity) {
                std::cout << "Error: Quantity exceeds integer limits.\n";
                return false;
            }
            existing->quantity += quantity;
        } else {
            head = new ResourceNode(name, quantity, head);
        }
        saveToFile();
        return true;
    }

    bool allocate(const std::string& name, int quantity, int& remainingQty) {
        if (name.empty() || quantity <= 0) return false;
        ResourceNode* existing = find(name);
        if (!existing) {
            std::cout << "Resource \"" << name << "\" not found in inventory.\n";
            return false;
        }
        if (quantity > existing->quantity) {
            std::cout << "Insufficient stock. Available: " << existing->quantity << "\n";
            return false;
        }
        existing->quantity -= quantity;
        remainingQty = existing->quantity;
        saveToFile();
        return true;
    }

    void display() const {
        if (!head) {
            std::cout << "\nResource inventory is currently empty.\n";
            return;
        }
        std::cout << "\n--- AVAILABLE RESOURCE INVENTORY ---\n";
        std::cout << "------------------------------------\n";
        ResourceNode* curr = head;
        while (curr) {
            std::cout << "• " << curr->name << ": " << curr->quantity << " units\n";
            curr = curr->next;
        }
        std::cout << "------------------------------------\n";
    }

    void saveToFile(const std::string& filename = "data/resources.txt") const {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) return;
        for (ResourceNode* curr = head; curr; curr = curr->next) {
            outFile << curr->name << "|" << curr->quantity << "\n";
        }
    }

    void loadFromFile(const std::string& filename = "data/resources.txt") {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) return;
        clear();
        std::string line;
        while (std::getline(inFile, line)) {
            if (line.empty()) continue;
            size_t delimPos = line.rfind('|');
            if (delimPos == std::string::npos) continue;
            std::string name = line.substr(0, delimPos);
            int qty = 0;
            try { qty = std::stoi(line.substr(delimPos + 1)); } catch (...) { continue; }
            if (qty >= 0) head = new ResourceNode(name, qty, head);
        }
    }
};

/* =========================================================================
 * 3. QUEUE: RESCUE REQUESTS (Linked Queue, FIFO)
 * ========================================================================= */
struct RescueRequest {
    int id;
    std::string name;
    std::string location;
    std::string emergency;
    int people;
    RescueRequest* next;

    RescueRequest(int reqId, const std::string& reqName, const std::string& loc,
                  const std::string& emerg, int count, RescueRequest* nextPtr = nullptr)
        : id(reqId), name(reqName), location(loc), emergency(emerg), people(count), next(nextPtr) {}
};

class RescueQueue {
private:
    RescueRequest* front;
    RescueRequest* rear;
    int nextRequestId;

    void clear() {
        while (front) {
            RescueRequest* temp = front;
            front = front->next;
            delete temp;
        }
        rear = nullptr;
    }

public:
    RescueQueue() : front(nullptr), rear(nullptr), nextRequestId(1) {}
    ~RescueQueue() { clear(); }

    RescueQueue(const RescueQueue&) = delete;
    RescueQueue& operator=(const RescueQueue&) = delete;

    int enqueue(const std::string& name, const std::string& location,
                const std::string& emergency, int people) {
        if (name.empty() || location.empty() || people <= 0) return -1;
        int newId = nextRequestId++;
        RescueRequest* newNode = new RescueRequest(newId, name, location, emergency, people);
        if (!front) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        saveToFile();
        return newId;
    }

    bool dequeue(RescueRequest& processedRequest) {
        if (!front) return false;
        RescueRequest* temp = front;
        processedRequest = *temp;
        front = front->next;
        if (!front) rear = nullptr;
        delete temp;
        saveToFile();
        return true;
    }

    void display() const {
        if (!front) {
            std::cout << "\nNo pending rescue requests in the queue.\n";
            return;
        }
        std::cout << "\n--- PENDING RESCUE REQUESTS (FIFO / Arrival Order) ---\n";
        std::cout << "-----------------------------------------------------\n";
        RescueRequest* curr = front;
        while (curr) {
            std::cout << "ID: #" << curr->id
                      << " | Name: " << curr->name
                      << " | Location: " << curr->location
                      << " | Emergency: " << curr->emergency
                      << " | People: " << curr->people << "\n";
            curr = curr->next;
        }
        std::cout << "-----------------------------------------------------\n";
    }

    void saveToFile(const std::string& filename = "data/requests.txt") const {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) return;
        for (RescueRequest* curr = front; curr; curr = curr->next) {
            outFile << curr->id << "|" << curr->name << "|" << curr->location
                    << "|" << curr->emergency << "|" << curr->people << "\n";
        }
    }

    void loadFromFile(const std::string& filename = "data/requests.txt") {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) return;
        clear();
        std::string line;
        while (std::getline(inFile, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> parts;
            while (std::getline(ss, token, '|')) parts.push_back(token);
            if (parts.size() != 5) continue;
            int id = 0, people = 0;
            try {
                id = std::stoi(parts[0]);
                people = std::stoi(parts[4]);
            } catch (...) { continue; }
            RescueRequest* newNode = new RescueRequest(id, parts[1], parts[2], parts[3], people);
            if (!front) front = rear = newNode;
            else { rear->next = newNode; rear = newNode; }
            if (id >= nextRequestId) nextRequestId = id + 1;
        }
    }
};

/* =========================================================================
 * 4. GRAPH: LOCATIONS & ROADS (Adjacency Matrix, BFS & DFS)
 * ========================================================================= */
class LocationGraph {
private:
    std::string locations[MAX_LOCATIONS];
    int adj[MAX_LOCATIONS][MAX_LOCATIONS];
    int locationCount;

    void dfsRecursive(int u, bool visited[]) const {
        visited[u] = true;
        std::cout << locations[u] << " -> ";
        for (int i = 0; i < locationCount; ++i) {
            if (adj[u][i] && !visited[i]) {
                dfsRecursive(i, visited);
            }
        }
    }

public:
    LocationGraph() : locationCount(0) {
        std::memset(adj, 0, sizeof(adj));
    }

    int findLocation(const std::string& name) const {
        for (int i = 0; i < locationCount; ++i) {
            if (locations[i] == name) return i;
        }
        return -1;
    }

    bool addLocation(const std::string& name) {
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

    bool addRoad(const std::string& first, const std::string& second) {
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

    void bfs(const std::string& startName) const {
        int start = findLocation(startName);
        if (start < 0) {
            std::cout << "Starting location \"" << startName << "\" not found.\n";
            return;
        }
        bool visited[MAX_LOCATIONS] = {false};
        int queue[MAX_LOCATIONS], head = 0, tail = 0;
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

    void dfs(const std::string& startName) const {
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

    void saveToFile(const std::string& locFile = "data/locations.txt",
                    const std::string& roadFile = "data/roads.txt") const {
        std::ofstream outLoc(locFile);
        if (outLoc.is_open()) {
            outLoc << locationCount << "\n";
            for (int i = 0; i < locationCount; ++i) outLoc << locations[i] << "\n";
        }
        std::ofstream outRoad(roadFile);
        if (outRoad.is_open()) {
            for (int i = 0; i < locationCount; ++i) {
                for (int j = i + 1; j < locationCount; ++j) {
                    if (adj[i][j]) outRoad << i << " " << j << "\n";
                }
            }
        }
    }

    void loadFromFile(const std::string& locFile = "data/locations.txt",
                      const std::string& roadFile = "data/roads.txt") {
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
                    if (std::getline(inLoc, name) && !name.empty()) {
                        locations[locationCount++] = name;
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
};

/* =========================================================================
 * 5. MAIN CONTROLLER & INTERACTIVE MENU
 * ========================================================================= */
void ensureDataDirectory() {
    try {
        if (!fs::exists("data")) fs::create_directories("data");
    } catch (...) {}
}

std::string getCleanLine(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, input)) return "";
        size_t first = input.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            std::cout << "Input cannot be empty. Please try again.\n";
            continue;
        }
        size_t last = input.find_last_not_of(" \t\r\n");
        std::string trimmed = input.substr(first, (last - first + 1));
        if (trimmed.find('|') != std::string::npos) {
            std::cout << "Character '|' is reserved. Please omit it.\n";
            continue;
        }
        return trimmed;
    }
}

int getPositiveInt(const std::string& prompt) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value > 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cout << "Invalid input. Please enter a positive integer.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

int main() {
    ensureDataDirectory();

    RescueQueue rescueQueue;
    ResourceManager resourceManager;
    LocationGraph locationGraph;
    OperationHistory history;

    rescueQueue.loadFromFile();
    resourceManager.loadFromFile();
    locationGraph.loadFromFile();
    history.loadFromFile();

    int choice = -1;
    do {
        std::cout << "\n====================================\n";
        std::cout << " DISASTER RESCUE & RESOURCE SYSTEM\n";
        std::cout << "====================================\n";
        std::cout << "1. Add rescue request\n";
        std::cout << "2. View pending requests\n";
        std::cout << "3. Process next request\n";
        std::cout << "4. Add/replenish resource\n";
        std::cout << "5. View resource inventory\n";
        std::cout << "6. Allocate resource\n";
        std::cout << "7. Add location\n";
        std::cout << "8. Add road\n";
        std::cout << "9. BFS/DFS traversal\n";
        std::cout << "10. View operation history\n";
        std::cout << "0. Save and exit\n";
        std::cout << "Enter choice: ";

        if (!(std::cin >> choice)) {
            std::cout << "Please enter a valid number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                std::string name = getCleanLine("Person/group name: ");
                std::string loc = getCleanLine("Affected location: ");
                std::string emergency = getCleanLine("Emergency type: ");
                int people = getPositiveInt("Number of people: ");
                int reqId = rescueQueue.enqueue(name, loc, emergency, people);
                if (reqId > 0) {
                    history.push("Request #" + std::to_string(reqId) + " added (" + name + " at " + loc + ")");
                    std::cout << "Request #" << reqId << " added to the queue.\n";
                }
                break;
            }
            case 2: rescueQueue.display(); break;
            case 3: {
                RescueRequest req(0, "", "", "", 0);
                if (rescueQueue.dequeue(req)) {
                    std::cout << "Processing request #" << req.id << " for " << req.name
                              << " at " << req.location << " (" << req.people << " people).\n";
                    history.push("Processed request #" + std::to_string(req.id));
                    std::cout << "Request removed from pending queue.\n";
                } else {
                    std::cout << "No pending requests in the queue.\n";
                }
                break;
            }
            case 4: {
                std::string name = getCleanLine("Resource name: ");
                int qty = getPositiveInt("Quantity to add: ");
                if (resourceManager.addOrReplenish(name, qty)) {
                    history.push("Stock updated: " + name + " (+" + std::to_string(qty) + ")");
                    std::cout << "Resource stock updated successfully.\n";
                }
                break;
            }
            case 5: resourceManager.display(); break;
            case 6: {
                std::string name = getCleanLine("Resource to allocate: ");
                int qty = getPositiveInt("Quantity to allocate: ");
                int remaining = 0;
                if (resourceManager.allocate(name, qty, remaining)) {
                    history.push("Allocated " + std::to_string(qty) + " " + name);
                    std::cout << "Allocation successful. Remaining " << name << ": " << remaining << "\n";
                }
                break;
            }
            case 7: {
                std::string name = getCleanLine("New location name: ");
                if (locationGraph.addLocation(name)) {
                    history.push("Location added: " + name);
                    std::cout << "Location added successfully.\n";
                }
                break;
            }
            case 8: {
                std::string first = getCleanLine("First location: ");
                std::string second = getCleanLine("Second location: ");
                if (locationGraph.addRoad(first, second)) {
                    history.push("Road added: " + first + " - " + second);
                    std::cout << "Road connected successfully.\n";
                }
                break;
            }
            case 9: {
                std::string start = getCleanLine("Starting location: ");
                std::cout << "1. BFS\n2. DFS\nChoice: ";
                int tChoice = 0;
                if (std::cin >> tChoice) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    if (tChoice == 1) locationGraph.bfs(start);
                    else if (tChoice == 2) locationGraph.dfs(start);
                    else std::cout << "Invalid traversal option.\n";
                } else {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid choice.\n";
                }
                break;
            }
            case 10: history.display(); break;
            case 0:
                rescueQueue.saveToFile();
                resourceManager.saveToFile();
                locationGraph.saveToFile();
                history.saveToFile();
                std::cout << "All data saved successfully. Goodbye!\n";
                break;
            default:
                std::cout << "Invalid choice. Please select from the menu (0-10).\n";
                break;
        }
    } while (choice != 0);

    return 0;
}
