#include "rescue_queue.h"
#include <iostream>
#include <fstream>
#include <sstream>

RescueQueue::RescueQueue() : front(nullptr), rear(nullptr), nextRequestId(1) {}

RescueQueue::~RescueQueue() {
    clearAll();
}

void RescueQueue::clearAll() {
    while (front) {
        RescueRequest* temp = front;
        front = front->next;
        delete temp;
    }
    rear = nullptr;
}

int RescueQueue::enqueue(const std::string& name, const std::string& location,
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

bool RescueQueue::dequeue(RescueRequest& processedRequest) {
    if (!front) return false;

    RescueRequest* temp = front;
    processedRequest = *temp; // copy data

    front = front->next;
    if (!front) {
        rear = nullptr;
    }

    delete temp;
    saveToFile();
    return true;
}

void RescueQueue::display() const {
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

bool RescueQueue::isEmpty() const {
    return front == nullptr;
}

int RescueQueue::getNextRequestId() const {
    return nextRequestId;
}

int RescueQueue::getCount() const {
    int count = 0;
    for (RescueRequest* curr = front; curr; curr = curr->next) {
        count++;
    }
    return count;
}

std::vector<RescueRequest> RescueQueue::getAllRequests() const {
    std::vector<RescueRequest> list;
    for (RescueRequest* curr = front; curr; curr = curr->next) {
        list.push_back(*curr);
    }
    return list;
}

void RescueQueue::saveToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) return;

    for (RescueRequest* curr = front; curr; curr = curr->next) {
        outFile << curr->id << "|"
                << curr->name << "|"
                << curr->location << "|"
                << curr->emergency << "|"
                << curr->people << "\n";
    }
}

void RescueQueue::loadFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) return;

    clearAll();
    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> parts;
        while (std::getline(ss, token, '|')) {
            parts.push_back(token);
        }

        if (parts.size() != 5) continue;

        int id = 0, people = 0;
        try {
            id = std::stoi(parts[0]);
            people = std::stoi(parts[4]);
        } catch (...) {
            continue;
        }

        RescueRequest* newNode = new RescueRequest(id, parts[1], parts[2], parts[3], people);
        if (!front) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }

        if (id >= nextRequestId) {
            nextRequestId = id + 1;
        }
    }
}
