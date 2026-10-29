#ifndef RESCUE_QUEUE_H
#define RESCUE_QUEUE_H

#include <string>
#include <vector>

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

public:
    RescueQueue();
    ~RescueQueue();

    // Prevent copying
    RescueQueue(const RescueQueue&) = delete;
    RescueQueue& operator=(const RescueQueue&) = delete;

    int enqueue(const std::string& name, const std::string& location,
                const std::string& emergency, int people);
    bool dequeue(RescueRequest& processedRequest);
    void display() const;
    bool isEmpty() const;
    int getNextRequestId() const;
    int getCount() const;
    void clearAll();

    std::vector<RescueRequest> getAllRequests() const;

    void saveToFile(const std::string& filename = "data/requests.txt") const;
    void loadFromFile(const std::string& filename = "data/requests.txt");
};

#endif // RESCUE_QUEUE_H
