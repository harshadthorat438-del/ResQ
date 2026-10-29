#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include "../structures/Queue.h"
#include "../structures/Stack.h"
#include "../structures/LinkedList.h"
#include "../structures/Graph.h"
#include "../structures/PriorityQueue.h"
#include "../structures/Sorting.h"
#include "../services/RescueService.h"
#include "../services/ResourceService.h"
#include "../services/NetworkService.h"
#include "../services/HistoryService.h"

int g_passedTests = 0;
int g_failedTests = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "  [FAIL] " << msg << " (Line " << __LINE__ << ")\n"; \
            ++g_failedTests; \
        } else { \
            std::cout << "  [PASS] " << msg << "\n"; \
            ++g_passedTests; \
        } \
    } while (0)

void testQueue() {
    std::cout << "\n--- Testing structures::Queue (FIFO) ---\n";
    structures::Queue<std::string> q;
    TEST_ASSERT(q.isEmpty(), "New queue is empty");

    q.enqueue("Req-1");
    q.enqueue("Req-2");
    q.enqueue("Req-3");
    TEST_ASSERT(q.size() == 3, "Queue size is 3");
    TEST_ASSERT(q.dequeue() == "Req-1", "First dequeued is Req-1 (FIFO order)");
    TEST_ASSERT(q.dequeue() == "Req-2", "Second dequeued is Req-2");
    TEST_ASSERT(q.dequeue() == "Req-3", "Third dequeued is Req-3");
    TEST_ASSERT(q.isEmpty(), "Queue is empty after dequeues");
}

void testPriorityQueue() {
    std::cout << "\n--- Testing structures::PriorityQueue (Max-Heap Triage) ---\n";
    structures::PriorityQueue<std::string> pq;
    TEST_ASSERT(pq.isEmpty(), "New PriorityQueue is empty");

    // Insert SIH Disaster Zones with Severity Scores
    pq.insert("Zone D (3 people)", 76.0);
    pq.insert("Zone B (8 people)", 160.0);
    pq.insert("Zone A (25 people)", 660.0);
    pq.insert("Zone C (40 people)", 1500.0);

    TEST_ASSERT(pq.size() == 4, "PriorityQueue size is 4");
    TEST_ASSERT(pq.peek() == "Zone C (40 people)", "Peek returns highest-risk Zone C (score 1500)");

    // Highest-risk zone must be extracted first (Triage!)
    std::string first = pq.extractMax();
    TEST_ASSERT(first == "Zone C (40 people)", "First extracted is Zone C (Highest Risk)");

    std::string second = pq.extractMax();
    TEST_ASSERT(second == "Zone A (25 people)", "Second extracted is Zone A (Score 660)");

    std::string third = pq.extractMax();
    TEST_ASSERT(third == "Zone B (8 people)", "Third extracted is Zone B (Score 160)");

    std::string fourth = pq.extractMax();
    TEST_ASSERT(fourth == "Zone D (3 people)", "Fourth extracted is Zone D (Score 76)");

    TEST_ASSERT(pq.isEmpty(), "PriorityQueue is empty after extracting all elements");
}

void testSorting() {
    std::cout << "\n--- Testing structures::Sorting (MergeSort) ---\n";
    struct ZoneItem {
        std::string name;
        int casualties;
    };

    std::vector<ZoneItem> zones = {
        {"Zone A", 25},
        {"Zone B", 8},
        {"Zone C", 40},
        {"Zone D", 3}
    };

    // Sort descending by casualty count using manual MergeSort
    structures::Sorting::mergeSort(zones, [](const ZoneItem& a, const ZoneItem& b) {
        return a.casualties > b.casualties;
    });

    TEST_ASSERT(zones[0].name == "Zone C" && zones[0].casualties == 40, "Rank 1 is Zone C (40 casualties)");
    TEST_ASSERT(zones[1].name == "Zone A" && zones[1].casualties == 25, "Rank 2 is Zone A (25 casualties)");
    TEST_ASSERT(zones[2].name == "Zone B" && zones[2].casualties == 8,  "Rank 3 is Zone B (8 casualties)");
    TEST_ASSERT(zones[3].name == "Zone D" && zones[3].casualties == 3,  "Rank 4 is Zone D (3 casualties)");
}

void testDijkstra() {
    std::cout << "\n--- Testing Dijkstra's Shortest Path Algorithm ---\n";
    structures::Graph g;
    g.addVertex("Central Depot");
    g.addVertex("Zone A");
    g.addVertex("Zone B");
    g.addVertex("Zone C");
    g.addVertex("Zone D");

    g.addEdge("Central Depot", "Zone A", "RD-1", 8.5);
    g.addEdge("Central Depot", "Zone B", "RD-2", 12.0);
    g.addEdge("Zone A", "Zone C", "RD-3", 5.2);
    g.addEdge("Zone B", "Zone C", "RD-4", 6.8);
    g.addEdge("Zone B", "Zone D", "RD-5", 4.1);

    // Compute shortest path from Central Depot to Zone C
    // Route 1: Central Depot -> Zone A -> Zone C = 8.5 + 5.2 = 13.7 min
    // Route 2: Central Depot -> Zone B -> Zone C = 12.0 + 6.8 = 18.8 min
    auto result = g.dijkstra("Central Depot", "Zone C");

    TEST_ASSERT(result.isReachable, "Zone C is reachable from Central Depot");
    TEST_ASSERT(result.path.size() == 3, "Shortest path consists of 3 nodes");
    TEST_ASSERT(result.path[0] == "Central Depot", "Path starts at Central Depot");
    TEST_ASSERT(result.path[1] == "Zone A", "Path traverses Zone A");
    TEST_ASSERT(result.path[2] == "Zone C", "Path ends at Zone C");
    TEST_ASSERT(result.totalDistance > 13.6 && result.totalDistance < 13.8, "Shortest time is 13.7 min");

    // Block RD-3 (Zone A <-> Zone C)
    std::cout << "  [ACTION] Marking RD-3 as 'Blocked'...\n";
    g.updateEdgeStatus("RD-3", "Blocked");

    // Recompute: Dijkstra must bypass blocked RD-3 and choose alternate route via Zone B
    auto bypassResult = g.dijkstra("Central Depot", "Zone C");
    TEST_ASSERT(bypassResult.isReachable, "Zone C is reachable via alternate open route");
    TEST_ASSERT(bypassResult.path.size() == 3, "Alternate path consists of 3 nodes");
    TEST_ASSERT(bypassResult.path[1] == "Zone B", "Dijkstra correctly avoids blocked road and routes via Zone B");
    TEST_ASSERT(bypassResult.totalDistance > 18.7 && bypassResult.totalDistance < 18.9, "Bypass distance is 18.8 min");

    // Block RD-4 as well -> Zone C now disconnected / unreachable
    std::cout << "  [ACTION] Marking RD-4 as 'Blocked' (Destination disconnected)...\n";
    g.updateEdgeStatus("RD-4", "Blocked");
    auto unreachResult = g.dijkstra("Central Depot", "Zone C");
    TEST_ASSERT(!unreachResult.isReachable, "Dijkstra reports destination unreachable when all paths blocked");
}

void testStack() {
    std::cout << "\n--- Testing structures::Stack (LIFO) ---\n";
    structures::Stack<std::string> s;
    s.push("Op-1");
    s.push("Op-2");
    s.push("Op-3");
    TEST_ASSERT(s.pop() == "Op-3", "LIFO pop returns top element");
    TEST_ASSERT(s.pop() == "Op-2", "Second pop returns middle element");
    TEST_ASSERT(s.pop() == "Op-1", "Third pop returns bottom element");
}

void testLinkedList() {
    std::cout << "\n--- Testing structures::LinkedList ---\n";
    structures::LinkedList<std::string> list;
    list.append("Ambulance Unit 1");
    list.append("Rescue Squad Alpha");
    TEST_ASSERT(list.size() == 2, "LinkedList size is 2");

    std::string* found = list.find([](const std::string& item) { return item == "Ambulance Unit 1"; });
    TEST_ASSERT(found != nullptr, "Found Ambulance Unit 1 in Linked List");
}

int main() {
    std::cout << "============================================================\n";
    std::cout << "  SIH / FDS DATA STRUCTURES & ALGORITHMS UNIT TEST SUITE\n";
    std::cout << "============================================================\n";

    testQueue();
    testPriorityQueue();
    testSorting();
    testDijkstra();
    testStack();
    testLinkedList();

    std::cout << "\n============================================================\n";
    std::cout << "  TEST SUMMARY: " << g_passedTests << " PASSED, " 
              << g_failedTests << " FAILED\n";
    std::cout << "============================================================\n";

    return (g_failedTests == 0) ? 0 : 1;
}
