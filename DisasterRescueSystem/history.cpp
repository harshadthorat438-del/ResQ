#include "history.h"
#include <iostream>
#include <fstream>

OperationHistory::OperationHistory() : historyTop(-1) {}

void OperationHistory::clearAll() {
    historyTop = -1;
}

void OperationHistory::push(const std::string& message) {
    if (message.empty()) return;

    if (historyTop == MAX_HISTORY - 1) {
        // Shift left to drop oldest entry when stack is at capacity
        for (int i = 1; i < MAX_HISTORY; ++i) {
            history[i - 1] = history[i];
        }
        historyTop--;
    }

    history[++historyTop] = message;
    saveToFile();
}

bool OperationHistory::pop(std::string& outMessage) {
    if (isEmpty()) return false;
    outMessage = history[historyTop--];
    saveToFile();
    return true;
}

void OperationHistory::display() const {
    if (isEmpty()) {
        std::cout << "\nNo operations recorded yet.\n";
        return;
    }

    std::cout << "\n--- RECENT OPERATIONS (LIFO / Most Recent First) ---\n";
    for (int i = historyTop; i >= 0; --i) {
        std::cout << "[" << (historyTop - i + 1) << "] " << history[i] << "\n";
    }
}

bool OperationHistory::isEmpty() const {
    return historyTop == -1;
}

int OperationHistory::size() const {
    return historyTop + 1;
}

std::vector<std::string> OperationHistory::getAllHistory() const {
    std::vector<std::string> list;
    // Return in LIFO order (top down to 0)
    for (int i = historyTop; i >= 0; --i) {
        list.push_back(history[i]);
    }
    return list;
}

void OperationHistory::saveToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        return;
    }

    for (int i = 0; i <= historyTop; ++i) {
        outFile << history[i] << "\n";
    }
}

void OperationHistory::loadFromFile(const std::string& filename) {
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
