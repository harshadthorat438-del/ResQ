#ifndef RESOURCES_H
#define RESOURCES_H

#include <string>
#include <vector>
#include <utility>

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

public:
    ResourceManager();
    ~ResourceManager();

    // Prevent copying to ensure safe memory management
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    bool addOrReplenish(const std::string& name, int quantity);
    bool allocate(const std::string& name, int quantity, int& remainingQty);
    ResourceNode* find(const std::string& name) const;
    void display() const;
    bool isEmpty() const;
    int getCount() const;
    int getTotalQuantity() const;
    void clearAll();

    std::vector<std::pair<std::string, int>> getAllResources() const;

    void saveToFile(const std::string& filename = "data/resources.txt") const;
    void loadFromFile(const std::string& filename = "data/resources.txt");
};

#endif // RESOURCES_H
