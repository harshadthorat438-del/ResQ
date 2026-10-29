# Disaster Rescue & Management System
### Second-Year Computer Science / IoT Engineering • Fundamentals of Data Structures (DSA)

> A mission-critical, full-stack disaster response and resource allocation system.  
> Powered by a **native C++17 REST backend** implementing manual data structures (Queue, Stack, Linked List, Binary Heap Priority Queue, Adjacency List Graph, MergeSort, and Dijkstra), paired with a responsive emergency management **web dashboard**.

---

## 1. System Architecture & Folder Structure

```
d:\FDS PROJECT\
├── main.cpp                     # C++17 Server entrypoint & graceful signal handler
├── build.bat                    # One-click Windows MinGW compilation script
├── run.bat                      # One-click launch script (compiles, starts backend, opens browser)
├── disaster_server.exe          # Native compiled high-performance REST server
├── api\
│   └── ApiServer.h              # REST API Router, endpoints & JSON serialization
├── models\
│   ├── Request.h                # Disaster report model with urgency score & status workflow
│   ├── RescueTeam.h             # Rescue responder team model with capability & location
│   ├── Resource.h               # Warehouse relief resource model with threshold & shortages
│   ├── Allocation.h             # Resource allocation audit record model
│   ├── Location.h               # Network node model (stations, affected villages, hospitals)
│   ├── Road.h                   # Bidirectional road model with travel times & blocked status
│   └── Operation.h              # Audit trail log model
├── structures\                  # MANUAL DSA IMPLEMENTATIONS (No STL container substitutes)
│   ├── LinkedList.h             # Singly linked list with deep copy, O(1) append & O(N) search
│   ├── PriorityQueue.h          # Binary Max-Heap priority queue with O(log N) siftUp/siftDown
│   ├── Queue.h                  # Node-based linked FIFO Queue with O(1) enqueue/dequeue
│   ├── Stack.h                  # Node-based linked LIFO Stack with O(1) push/pop
│   ├── Graph.h                  # Adjacency List graph with BFS, DFS & Dijkstra algorithm
│   └── Sorting.h                # Divide-and-Conquer stable MergeSort (O(N log N))
├── services\
│   ├── DatabaseService.h        # SQLite C-API database engine with 8 tables, indexes & transactions
│   ├── RescueService.h          # Disaster reports linked list & binary max-heap triage
│   ├── ResourceService.h        # Warehouse stock linked list & anti-shortage safeguards
│   ├── NetworkService.h         # Road network graph, road block toggling & reachability
│   ├── HistoryService.h         # LIFO Stack operation history audit service
│   └── StorageService.h         # Safe text file persistence engine with pipe serialization
├── data\                        # Persistent database and text backup files
│   ├── disaster_system.db       # Relational SQLite database file (ACID transactional storage)
│   ├── requests.txt             # Saved disaster reports
│   ├── teams.txt                # Saved rescue team fleet records
│   ├── resources.txt            # Saved relief inventory stock
│   ├── allocations.txt          # Saved resource distribution records
│   ├── locations.txt            # Saved network locations
│   ├── roads.txt                # Saved road connections & blocked statuses
│   └── history.txt              # Saved operation audit ledger
├── frontend\
│   ├── index.html               # Humanitarian homepage & operator coordination dashboard
│   ├── css\style.css            # Professional humanitarian palette & Lucide vector styling
│   └── js\
│       ├── api.js               # REST client for backend communication
│       ├── graph.js             # SVG road network visualizer with vector badges
│       └── app.js               # Main operational controller & workflow engine
├── tests\
│   ├── test_structures.cpp      # Manual DSA unit test suite (34/34 tests passed)
│   └── test_database.cpp        # SQLite database unit test suite (10/10 tests passed)
└── third_party\
    ├── sqlite3.h / sqlite3.c    # Official SQLite amalgamation (v3.54.1)
    ├── sqlite3.o                # Precompiled SQLite C object file
    ├── httplib.h                # Lightweight C++17 HTTP/REST server library
    └── json.hpp                 # nlohmann/json C++ serialization library
```

---

## 2. Core Data Structures & Algorithms (DSA Reference)

| Data Structure / Algorithm | System Module | Responsibility in Disaster System | Time Complexity | Space Complexity | Custom C++ Source File |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Singly Linked List** | Disaster Reports | Stores and manages disaster incident reports dynamically | Insert: $\mathcal{O}(1)$<br>Search/Update: $\mathcal{O}(N)$ | $\mathcal{O}(N)$ | [`structures/LinkedList.h`](file:///d:/FDS%20PROJECT/structures/LinkedList.h) |
| **Binary Max-Heap** | Priority Queue | Ranks emergencies dynamically by composite triage urgency | Insert: $\mathcal{O}(\log N)$<br>ExtractMax: $\mathcal{O}(\log N)$<br>Peek: $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | [`structures/PriorityQueue.h`](file:///d:/FDS%20PROJECT/structures/PriorityQueue.h) |
| **Linked Queue** | Dispatch Management | Manages pending dispatch scheduling in priority-ranked order | Enqueue: $\mathcal{O}(1)$<br>Dequeue: $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | [`structures/Queue.h`](file:///d:/FDS%20PROJECT/structures/Queue.h) |
| **Singly Linked List** | Resources & Fleet | Maintains dynamically addable resources and rescue team nodes | Append: $\mathcal{O}(1)$<br>Find/Update: $\mathcal{O}(N)$ | $\mathcal{O}(N)$ | [`structures/LinkedList.h`](file:///d:/FDS%20PROJECT/structures/LinkedList.h) |
| **Adjacency List Graph**| Road Network | Sparse representation of rescue stations, affected villages & roads | Add Vertex: $\mathcal{O}(1)$<br>Add Edge: $\mathcal{O}(\deg(u))$ | $\mathcal{O}(V + E)$ | [`structures/Graph.h`](file:///d:/FDS%20PROJECT/structures/Graph.h) |
| **Dijkstra's Algorithm**| Shortest Rescue Route| Computes shortest transit time avoiding blocked roads | $\mathcal{O}((V + E) \log V)$ | $\mathcal{O}(V)$ | [`structures/Graph.h`](file:///d:/FDS%20PROJECT/structures/Graph.h) |
| **BFS & DFS** | Network Exploration | Explores reachable operational zones and traversal order | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ | [`structures/Graph.h`](file:///d:/FDS%20PROJECT/structures/Graph.h) |
| **MergeSort** | Incident Ranking | Stable sorting of emergencies by casualty population | Best/Avg/Worst: $\mathcal{O}(N \log N)$ | $\mathcal{O}(N)$ | [`structures/Sorting.h`](file:///d:/FDS%20PROJECT/structures/Sorting.h) |
| **Linked Stack** | Operation History | Audit trail recording administrative actions (most recent first) | Push: $\mathcal{O}(1)$<br>Pop: $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | [`structures/Stack.h`](file:///d:/FDS%20PROJECT/structures/Stack.h) |

---

## 3. Standard Initial Demonstration Dataset (Section 8)

The initial dataset initializes automatically when the store is empty, or whenever the operator triggers **Reset Demo Data**:

### A. Network Locations (Graph Vertices)
- `Relief Camp`: Rescue Station $(x=120, y=250)$
- `Village A`: Affected Area $(x=330, y=140)$
- `Village B`: Affected Area $(x=330, y=360)$
- `Village C`: Affected Area $(x=580, y=140)$
- `Medical Center`: Rescue Station $(x=780, y=250)$

### B. Road Connections (Bidirectional Edges with Travel Times)
- `Relief Camp ↔ Village A`: $5.0\text{ minutes}$
- `Village A ↔ Village B`: $4.0\text{ minutes}$
- `Village A ↔ Village C`: $7.0\text{ minutes}$
- `Village B ↔ Medical Center`: $3.0\text{ minutes}$
- `Village C ↔ Medical Center`: $6.0\text{ minutes}$

### C. Rescue Responders (Linked List)
- `T01`: **General Rescue Alpha**, Capability: *General Rescue*, Station: *Relief Camp*, Status: *Available*
- `T02`: **Medical Team Beta**, Capability: *Medical Team*, Station: *Medical Center*, Status: *Available*
- `T03`: **Flood Rescue Gamma**, Capability: *Flood Rescue*, Station: *Village A*, Status: *Available*

### D. Relief Supplies Inventory (Linked List)
- **Medical Kits:** 50 units (Threshold: 20)
- **Food Packets:** 80 units (Threshold: 30)
- **Water Bottles:** 100 units (Threshold: 40)
- **Rescue Boats:** 5 units (Threshold: 2)

### E. Sample Incident Reports
- `R101`: *Commander Sharma*, `Village A`, Earthquake, Critical, 12 affected (8 immediate rescue), requires 15 Medical Kits & General Rescue.
- `R102`: *Sarpanch Patil*, `Village B`, Flood, High, 25 affected (10 immediate rescue), requires 2 Rescue Boats & Flood Rescue.
- `R103`: *Dr. Rao*, `Village C`, Medical Emergency, Critical, 3 affected (2 immediate rescue), requires 5 Medical Kits & Medical Team.

---

## 4. Urgency Triage Policy & Dijkstra Route Proof

### Documented Urgency Scoring Formula:
$$\text{Urgency Score} = \text{Severity Base} + (\text{Immediate Casualties} \times 15) + \text{Hazard Threat} + (\text{Affected Population} \times 2) + \text{Escalation}$$

1. **R102 (Village B - Flood):**  
   $\text{High (60)} + (10 \times 15 = 150) + \text{Flood (30)} + (25 \times 2 = 50) = \mathbf{290.0}$ $\longrightarrow$ **Rank #1**
2. **R101 (Village A - Earthquake):**  
   $\text{Critical (100)} + (8 \times 15 = 120) + \text{Earthquake (30)} + (12 \times 2 = 24) = \mathbf{274.0}$ $\longrightarrow$ **Rank #2**
3. **R103 (Village C - Medical Emergency):**  
   $\text{Critical (100)} + (2 \times 15 = 30) + \text{Medical (20)} + (3 \times 2 = 6) = \mathbf{156.0}$ $\longrightarrow$ **Rank #3**

### Shortest Route Routing & Blocked Road Avoidance Proof:
Transit from `Relief Camp` to `Medical Center`:
- **Default Route:** $\text{Relief Camp} \rightarrow \text{Village A} \rightarrow \text{Village B} \rightarrow \text{Medical Center} = 5 + 4 + 3 = \mathbf{12.0\text{ min}}$
- **Alternate Route via Village C:** $5 + 7 + 6 = \mathbf{18.0\text{ min}}$
- **When Road `RD-2` (Village A ↔ Village B) is Blocked:**  
  Dijkstra excludes `RD-2` and routes via Village C ($18.0\text{ min}$), reporting reachability.
- **When all paths to Medical Center are blocked (`RD-4` & `RD-5` blocked):**  
  Dijkstra reports `isReachable: false` with zero transit time and safe handling.

---

## 5. Execution Instructions (Windows)

### Prerequisites:
- Compiler: GCC / MinGW-w64 with C++17 support.
- Windows 10 or 11.
- No external runtime installations (Node.js or Python) required — native C++ executable serves both API and Web UI.

### Step 1: One-Click Build & Run
Double-click [`run.bat`](file:///d:/FDS%20PROJECT/run.bat) or run from PowerShell:
```powershell
.\run.bat
```
This builds `disaster_server.exe` (if not already compiled), starts the server on port `8080`, and opens `http://localhost:8080` in your default browser.

### Step 2: Manual Build (Optional)
```powershell
g++ -std=c++17 -Wall -Wextra -O2 main.cpp third_party/sqlite3.o -lws2_32 -o disaster_server.exe
.\disaster_server.exe 8080
```

### Step 3: Run Unit Test Suites
1. **Manual FDS Data Structures Suite (Queue, Max-Heap, MergeSort, Dijkstra, Stack, LinkedList):**
```powershell
g++ -std=c++17 -Wall -Wextra tests/test_structures.cpp -o test_structures.exe
.\test_structures.exe
```
*(Result: 34 tests passed, 0 failed).*

2. **SQLite Relational Database & Transactions Suite:**
```powershell
g++ -std=c++17 -Wall -Wextra tests/test_database.cpp third_party/sqlite3.o -lws2_32 -o test_database.exe
.\test_database.exe
```
*(Result: 10/10 tests passed, 0 failed).*

---

## 6. REST API Reference

| Endpoint | Method | Description |
| :--- | :---: | :--- |
| `/api/health` | `GET` | Health check, compiler information, and system status |
| `/api/stats` | `GET` | Live dashboard metrics calculated dynamically in C++ |
| `/api/reports` | `GET` | Search and filter disaster reports from Singly Linked List |
| `/api/reports` | `POST` | Register a new disaster emergency report |
| `/api/reports/escalate` | `POST` | Escalate incident urgency to Critical (+priority boost) |
| `/api/priority-queue` | `GET` | Retrieve emergencies sorted by urgency score via Binary Max-Heap |
| `/api/priority-queue/triage-next` | `POST` | Dispatch highest-risk emergency via `extractMax()` |
| `/api/priority-queue/fifo-next` | `POST` | Dispatch emergency in standard arrival order via FIFO Queue |
| `/api/teams` | `GET` | List all rescue responder teams in fleet |
| `/api/teams` | `POST` | Add a new rescue team to fleet |
| `/api/teams/release` | `POST` | Release team back to Available status |
| `/api/assignment/recommend` | `GET` | Preview team recommendation, travel time & resource check |
| `/api/assignment/confirm` | `POST` | Confirm assignment: mark team Busy, allocate stock, set Assigned |
| `/api/assignment/update-status` | `POST` | Transition status: In Progress $\rightarrow$ Completed / Cancelled |
| `/api/resources` | `GET` | List warehouse inventory with low-stock warnings |
| `/api/resources` | `POST` | Add new resource type or replenish stock units |
| `/api/resources/allocate` | `POST` | Allocate supplies with shortage detection safeguard |
| `/api/allocations` | `GET` | Retrieve past distribution history |
| `/api/locations` | `GET` | List network locations (graph vertices) |
| `/api/locations` | `POST` | Register a new location vertex |
| `/api/roads` | `GET` | List road connections (graph edges) |
| `/api/roads` | `POST` | Add a bidirectional road connection |
| `/api/roads/toggle-block` | `POST` | **Toggle road Blocked / Open status** |
| `/api/routes/dijkstra` | `GET` | Calculate shortest route avoiding blocked roads via Dijkstra |
| `/api/routes/bfs` | `GET` | Run Breadth-First Search reachability using FIFO Queue |
| `/api/routes/dfs` | `GET` | Run Depth-First Search exploration using LIFO Stack |
| `/api/history` | `GET` | Retrieve audit events from C++ LIFO Stack |
| `/api/save` | `POST` | Persist all application records to local `data/` files |
| `/api/demo` | `POST` | Reset Section 8 standardized demonstration dataset |
| `/api/export` | `GET` | Export complete application state as JSON backup |

---

## 7. Viva Q&A Guide for Engineering Students

### Q1: Why use an Adjacency List instead of an Adjacency Matrix for the road network?
**Answer:** Real-world road networks are sparse graphs where $|E| \ll |V|^2$. An Adjacency List requires $\mathcal{O}(V + E)$ memory, whereas an Adjacency Matrix wastes $\mathcal{O}(V^2)$ memory on non-existent road links. Furthermore, Dijkstra, BFS, and DFS can iterate over adjacent edges in $\mathcal{O}(\deg(u))$ time rather than scanning through all $V$ entries.

### Q2: Why is a Binary Max-Heap better than a sorted array for emergency triage?
**Answer:** A sorted array requires $\mathcal{O}(N)$ time to insert an emergency report. A Binary Max-Heap maintains complete binary tree invariants in contiguous memory and allows inserting new incidents in $\mathcal{O}(\log N)$ time and extracting the highest-urgency casualty zone via `extractMax()` in $\mathcal{O}(\log N)$ time, ensuring optimal response times even under high report volumes.

### Q3: How does Dijkstra's Algorithm handle blocked roads?
**Answer:** In [`structures/Graph.h`](file:///d:/FDS%20PROJECT/structures/Graph.h), during edge relaxation from vertex $u$ to neighbor $v$, the algorithm inspects `edge.status`. If `edge.status == "Blocked"`, the edge is skipped (`continue;`). If all paths to a destination are blocked, `dist[destination]` remains infinity ($\infty$), and `isReachable` evaluates to `false`.

### Q4: How does the system prevent double-booking of rescue teams?
**Answer:** In [`services/ResourceService.h`](file:///d:/FDS%20PROJECT/services/ResourceService.h), each rescue team node stores a `status` field ("Available" or "Busy"). When an operator confirms an assignment, `assignTeam()` verifies that `team->status == "Available"`. If the team is already marked "Busy", an exception is thrown, returning an HTTP 400 Bad Request and preventing duplicate assignment.
