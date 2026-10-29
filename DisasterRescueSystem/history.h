#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include <vector>

constexpr int MAX_HISTORY = 100;

class OperationHistory {
private:
    std::string history[MAX_HISTORY];
    int historyTop;

public:
    OperationHistory();

    void push(const std::string& message);
    bool pop(std::string& outMessage);
    void display() const;
    bool isEmpty() const;
    int size() const;
    int getCapacity() const { return MAX_HISTORY; }
    void clearAll();

    std::vector<std::string> getAllHistory() const;

    void saveToFile(const std::string& filename = "data/history.txt") const;
    void loadFromFile(const std::string& filename = "data/history.txt");
};

#endif // HISTORY_H
