# Disaster Rescue & Resource Management System
### Interactive Web Application & First-Year Engineering FDS Project

A full-stack emergency management dashboard and educational data structures visualizer powered by a **high-performance C++17 REST backend** and an **interactive HTML5/CSS3/JavaScript frontend**.

---

## 1. Data Structures & Architectural Mapping

The core data structures are strictly executed in **C++ on the backend** with real-time state synchronization to the web client:

| Module | Data Structure | Underlying Implementation | C++ Class / Struct | Visualization & Education |
| :--- | :--- | :--- | :--- | :--- |
| **Rescue Queue** | **Queue** | Linked Queue (`front` & `rear` pointers, FIFO) | [`RescueQueue`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/rescue_queue.h), [`RescueRequest`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/rescue_queue.h) | Interactive node cards showing `FRONT` and `REAR` pointers, dynamic enqueue/dequeue animations |
| **Relief Inventory** | **Linked List** | Dynamic singly linked list (`head` pointer) | [`ResourceManager`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/resources.h), [`ResourceNode`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/resources.h) | `[ Name \| Qty \| next* ] ──► [ NULL ]` chain showing node pointer links and dynamic heap allocation |
| **Locations & Roads** | **Graph** | Adjacency Matrix (`adj[20][20]`) | [`LocationGraph`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/locations.h) | Draggable interactive HTML Canvas displaying vertices, edges, and modal binary matrix view |
| **Route Traversals** | **BFS & DFS** | Queue-based BFS and Recursive DFS | [`LocationGraph::bfsWithTrace`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/locations.cpp), `dfsWithTrace` | Real-time step-by-step playback highlighting visited paths and inspecting the underlying **BFS Queue** or **DFS Call Stack** |
| **Operation Log** | **Stack** | Array-based Stack (LIFO) | [`OperationHistory`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/history.h) | Vertical stack visualizer with dynamic `TOP` pointer tracking recent operations |
| **File Persistence** | **Delimited Text** | C++ file streams (`data/`) | Automated file I/O | Long-term state preservation between runs |

---

## 2. Web Application Architecture

```text
DisasterRescueSystem/
├── server.cpp            # C++17 REST API server using cpp-httplib & json.hpp
├── rescue_queue.h/.cpp   # Linked Queue data structure implementation
├── resources.h/.cpp      # Singly Linked List data structure implementation
├── locations.h/.cpp      # Adjacency Matrix Graph with BFS/DFS step tracing
├── history.h/.cpp        # Array-based Stack implementation (LIFO)
├── build_server.bat      # Windows one-click server compilation script
├── run_server.bat        # Windows one-click server launcher with browser launch
├── public/               # Frontend Client Assets
│   ├── index.html        # Emergency command dashboard UI
│   ├── css/style.css     # Dark navy theme with teal/green/amber accents
│   ├── js/canvas.js      # Draggable canvas graph renderer & traversal animator
│   └── js/app.js         # API integration, UI event handlers, DS animators
└── data/                 # Auto-generated persistent storage
    ├── requests.txt
    ├── resources.txt
    ├── locations.txt
    ├── roads.txt
    └── history.txt
```

---

## 3. How to Run the Web Application

### Option A: Using the Launcher Script (Easiest)
Double-click [`run_server.bat`](file:///d:/FDS%20PROJECT/DisasterRescueSystem/run_server.bat) or run from PowerShell:
```powershell
cd "d:\FDS PROJECT\DisasterRescueSystem"
.\run_server.bat
```
This automatically starts `disaster_server.exe` on port `8080` and opens your default browser at `http://localhost:8080`.

### Option B: Compiling from Source
To recompile the server with GCC/MinGW:
```powershell
.\build_server.bat
.\disaster_server.exe
```

---

## 4. REST API Endpoints Reference

All endpoints return and accept JSON:

- `GET /api/status` — Live statistics overview (counts of requests, people, resources, stock, locations, roads).
- `GET /api/requests` — All requests in FIFO order from `front` to `rear`.
- `POST /api/requests` — Enqueue a new request `{"name":"...","location":"...","emergency":"...","people":5}`.
- `POST /api/requests/process` — Dequeue and process the request at `front`.
- `GET /api/resources` — All resources in the singly linked list starting from `head`.
- `POST /api/resources` — Add or replenish stock `{"name":"...","quantity":100}`.
- `POST /api/resources/allocate` — Allocate supplies `{"name":"...","quantity":20}`.
- `GET /api/locations` — All vertices, edges, and the full $20 \times 20$ adjacency matrix.
- `POST /api/locations` — Register a new location `{"name":"..."}`.
- `POST /api/roads` — Create a road connection `{"first":"...","second":"..."}`.
- `GET /api/traversal?start=<name>&type=<bfs|dfs>` — Executes BFS or DFS in C++ and returns step-by-step traces with node order, active queue/stack states, and educational explanations.
- `GET /api/history` — All operations in LIFO order from `top` down to 0.
- `POST /api/reset-demo` — Seeds standard disaster scenario preset.
