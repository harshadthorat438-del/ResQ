#include "resources.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>

ResourceManager::ResourceManager() : head(nullptr) {}

ResourceManager::~ResourceManager() {
    clearAll();
}

void ResourceManager::clearAll() {
    while (head) {
        ResourceNode* temp = head;
        head = head->next;
        delete temp;
    }
}

ResourceNode* ResourceManager::find(const std::string& name) const {
    ResourceNode* curr = head;
    while (curr) {
        if (curr->name == name) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

bool ResourceManager::addOrReplenish(const std::string& name, int quantity) {
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

bool ResourceManager::allocate(const std::string& name, int quantity, int& remainingQty) {
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

void ResourceManager::display() const {
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

bool ResourceManager::isEmpty() const {
    return head == nullptr;
}

int ResourceManager::getCount() const {
    int count = 0;
    for (ResourceNode* curr = head; curr; curr = curr->next) {
        count++;
    }
    return count;
}

int ResourceManager::getTotalQuantity() const {
    int total = 0;
    for (ResourceNode* curr = head; curr; curr = curr->next) {
        total += curr->quantity;
    }
    return total;
}

std::vector<std::pair<std::string, int>> ResourceManager::getAllResources() const {
    std::vector<std::pair<std::string, int>> list;
    for (ResourceNode* curr = head; curr; curr = curr->next) {
        list.push_back({curr->name, curr->quantity});
    }
    return list;
}

void ResourceManager::saveToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) return;

    for (ResourceNode* curr = head; curr; curr = curr->next) {
        outFile << curr->name << "|" << curr->quantity << "\n";
    }
}

void ResourceManager::loadFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) return;

    clearAll();
    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        size_t delimPos = line.rfind('|');
        if (delimPos == std::string::npos) continue;

        std::string name = line.substr(0, delimPos);
        int qty = 0;
        try {
            qty = std::stoi(line.substr(delimPos + 1));
        } catch (...) {
            continue;
        }

        if (qty >= 0) {
            head = new ResourceNode(name, qty, head);
        }
    }
}
