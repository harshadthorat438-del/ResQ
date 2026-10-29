#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOCATIONS 20
#define MAX_TEXT 100
#define MAX_HISTORY 100

/* ---------- QUEUE: RESCUE REQUESTS ---------- */

typedef struct Request {
    int id;
    char name[MAX_TEXT];
    char location[MAX_TEXT];
    char emergency[MAX_TEXT];
    int people;
    struct Request *next;
} Request;

Request *front = NULL, *rear = NULL;
int nextRequestId = 1;

void saveRequests(void) {
    FILE *fp = fopen("data/requests.txt", "w");
    if (!fp) {
        printf("Cannot save requests.\n");
        return;
    }

    for (Request *p = front; p; p = p->next)
        fprintf(fp, "%d|%s|%s|%s|%d\n",
                p->id, p->name, p->location,
                p->emergency, p->people);

    fclose(fp);
}

void loadRequests(void) {
    FILE *fp = fopen("data/requests.txt", "r");
    if (!fp) return;

    char line[400];
    while (fgets(line, sizeof(line), fp)) {
        Request *p = calloc(1, sizeof(Request));
        if (!p) break;

        if (sscanf(line, "%d|%99[^|]|%99[^|]|%99[^|]|%d",
                   &p->id, p->name, p->location,
                   p->emergency, &p->people) != 5) {
            free(p);
            continue;
        }

        p->next = NULL;
        if (!front) front = rear = p;
        else {
            rear->next = p;
            rear = p;
        }

        if (p->id >= nextRequestId)
            nextRequestId = p->id + 1;
    }
    fclose(fp);
}

void addRequest(void) {
    Request *p = calloc(1, sizeof(Request));
    if (!p) {
        printf("Memory allocation failed.\n");
        return;
    }

    p->id = nextRequestId++;

    printf("Person/group name (no | character): ");
    scanf(" %99[^\n]", p->name);

    printf("Affected location (no | character): ");
    scanf(" %99[^\n]", p->location);

    printf("Emergency type (no | character): ");
    scanf(" %99[^\n]", p->emergency);

    printf("Number of people: ");
    if (scanf("%d", &p->people) != 1 || p->people <= 0) {
        printf("Invalid number of people.\n");
        while (getchar() != '\n');
        free(p);
        return;
    }

    p->next = NULL;

    if (!front) front = rear = p;
    else {
        rear->next = p;
        rear = p;
    }

    saveRequests();
    printf("Request #%d added to the queue.\n", p->id);
}

void displayRequests(void) {
    Request *p = front;
    if (!p) {
        printf("No pending rescue requests.\n");
        return;
    }

    printf("\n--- PENDING RESCUE REQUESTS ---\n");
    while (p) {
        printf("ID: %d | Name: %s | Location: %s"
               " | Emergency: %s | People: %d\n",
               p->id, p->name, p->location,
               p->emergency, p->people);
        p = p->next;
    }
}

/* ---------- STACK: OPERATION HISTORY ---------- */

char history[MAX_HISTORY][MAX_TEXT];
int historyTop = -1;

void pushHistory(const char *message) {
    if (historyTop == MAX_HISTORY - 1) {
        for (int i = 1; i < MAX_HISTORY; i++)
            strcpy(history[i - 1], history[i]);
        historyTop--;
    }

    strcpy(history[++historyTop], message);

    FILE *fp = fopen("data/history.txt", "w");
    if (!fp) return;

    for (int i = 0; i <= historyTop; i++)
        fprintf(fp, "%s\n", history[i]);

    fclose(fp);
}

void loadHistory(void) {
    FILE *fp = fopen("data/history.txt", "r");
    if (!fp) return;

    char line[MAX_TEXT];
    while (historyTop < MAX_HISTORY - 1 &&
           fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        strcpy(history[++historyTop], line);
    }
    fclose(fp);
}

void displayHistory(void) {
    if (historyTop == -1) {
        printf("No operations recorded.\n");
        return;
    }

    printf("\n--- RECENT OPERATIONS (LIFO) ---\n");
    for (int i = historyTop; i >= 0; i--)
        printf("%s\n", history[i]);
}

/* ---------- LINKED LIST: RESOURCE INVENTORY ---------- */

typedef struct Resource {
    char name[MAX_TEXT];
    int quantity;
    struct Resource *next;
} Resource;

Resource *resourceHead = NULL;

void saveResources(void) {
    FILE *fp = fopen("data/resources.txt", "w");
    if (!fp) {
        printf("Cannot save resources.\n");
        return;
    }

    for (Resource *p = resourceHead; p; p = p->next)
        fprintf(fp, "%s|%d\n", p->name, p->quantity);

    fclose(fp);
}

void loadResources(void) {
    FILE *fp = fopen("data/resources.txt", "r");
    if (!fp) return;

    char line[250], name[MAX_TEXT];
    int quantity;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%99[^|]|%d", name, &quantity) != 2)
            continue;
        if (quantity < 0) continue;

        Resource *p = calloc(1, sizeof(Resource));
        if (!p) break;

        strcpy(p->name, name);
        p->quantity = quantity;
        p->next = resourceHead;
        resourceHead = p;
    }

    fclose(fp);
}

Resource *findResource(const char *name) {
    for (Resource *p = resourceHead; p; p = p->next)
        if (strcmp(p->name, name) == 0)
            return p;
    return NULL;
}

void addResource(void) {
    char name[MAX_TEXT];
    int quantity;

    printf("Resource name (no | character): ");
    scanf(" %99[^\n]", name);

    printf("Quantity to add: ");
    if (scanf("%d", &quantity) != 1 || quantity <= 0) {
        printf("Enter a positive quantity.\n");
        while (getchar() != '\n');
        return;
    }

    Resource *p = findResource(name);

    if (p) {
        if (quantity > 2147483647 - p->quantity) {
            printf("Quantity is too large.\n");
            return;
        }
        p->quantity += quantity;
    } else {
        p = calloc(1, sizeof(Resource));
        if (!p) {
            printf("Memory allocation failed.\n");
            return;
        }

        strcpy(p->name, name);
        p->quantity = quantity;
        p->next = resourceHead;
        resourceHead = p;
    }

    saveResources();

    char message[MAX_TEXT];
    snprintf(message, sizeof(message), "Stock updated: %s", name);
    pushHistory(message);

    printf("Resource stock updated.\n");
}

void displayResources(void) {
    if (!resourceHead) {
        printf("Inventory is empty.\n");
        return;
    }

    printf("\n--- AVAILABLE RESOURCES ---\n");
    for (Resource *p = resourceHead; p; p = p->next)
        printf("%s: %d\n", p->name, p->quantity);
}

void allocateResource(void) {
    char name[MAX_TEXT];
    int quantity;

    printf("Resource to allocate: ");
    scanf(" %99[^\n]", name);

    Resource *p = findResource(name);
    if (!p) {
        printf("Resource not found.\n");
        return;
    }

    printf("Quantity to allocate: ");
    if (scanf("%d", &quantity) != 1 || quantity <= 0) {
        printf("Enter a positive quantity.\n");
        while (getchar() != '\n');
        return;
    }

    if (quantity > p->quantity) {
        printf("Insufficient stock. Available: %d\n", p->quantity);
        return;
    }

    p->quantity -= quantity;
    saveResources();

    char message[MAX_TEXT];
    snprintf(message, sizeof(message), "Allocated %d %s",
             quantity, name);
    pushHistory(message);

    printf("Allocation successful. Remaining: %d\n", p->quantity);
}

/* ---------- GRAPH: LOCATIONS, ROADS, BFS, DFS ---------- */

char locations[MAX_LOCATIONS][MAX_TEXT];
int graph[MAX_LOCATIONS][MAX_LOCATIONS];
int locationCount = 0;

void saveGraph(void) {
    FILE *fp = fopen("data/locations.txt", "w");
    if (!fp) return;

    fprintf(fp, "%d\n", locationCount);
    for (int i = 0; i < locationCount; i++)
        fprintf(fp, "%s\n", locations[i]);

    fclose(fp);

    fp = fopen("data/roads.txt", "w");
    if (!fp) return;

    for (int i = 0; i < locationCount; i++)
        for (int j = i + 1; j < locationCount; j++)
            if (graph[i][j])
                fprintf(fp, "%d %d\n", i, j);

    fclose(fp);
}

void loadGraph(void) {
    FILE *fp = fopen("data/locations.txt", "r");
    if (!fp) return;

    int count;
    if (fscanf(fp, "%d\n", &count) != 1 ||
        count < 0 || count > MAX_LOCATIONS) {
        fclose(fp);
        return;
    }

    for (int i = 0; i < count; i++) {
        if (!fgets(locations[i], MAX_TEXT, fp)) break;
        locations[i][strcspn(locations[i], "\r\n")] = '\0';
        locationCount++;
    }
    fclose(fp);

    fp = fopen("data/roads.txt", "r");
    if (!fp) return;

    int a, b;
    while (fscanf(fp, "%d %d", &a, &b) == 2) {
        if (a >= 0 && b >= 0 && a < locationCount &&
            b < locationCount && a != b)
            graph[a][b] = graph[b][a] = 1;
    }
    fclose(fp);
}

void addLocation(void) {
    if (locationCount >= MAX_LOCATIONS) {
        printf("Location limit reached.\n");
        return;
    }

    char name[MAX_TEXT];
    printf("New location name: ");
    scanf(" %99[^\n]", name);

    for (int i = 0; i < locationCount; i++) {
        if (strcmp(locations[i], name) == 0) {
            printf("Location already exists.\n");
            return;
        }
    }

    strcpy(locations[locationCount++], name);
    saveGraph();

    char message[MAX_TEXT];
    snprintf(message, sizeof(message), "Location added: %s", name);
    pushHistory(message);

    printf("Location added.\n");
}

int findLocation(const char *name) {
    for (int i = 0; i < locationCount; i++)
        if (strcmp(locations[i], name) == 0)
            return i;
    return -1;
}

void addRoad(void) {
    char first[MAX_TEXT], second[MAX_TEXT];

    printf("First location: ");
    scanf(" %99[^\n]", first);
    printf("Second location: ");
    scanf(" %99[^\n]", second);

    int a = findLocation(first);
    int b = findLocation(second);

    if (a < 0 || b < 0 || a == b) {
        printf("Invalid locations. Add both locations first.\n");
        return;
    }

    graph[a][b] = graph[b][a] = 1;
    saveGraph();

    char message[MAX_TEXT];
    snprintf(message, sizeof(message), "Road added: %s - %s",
             first, second);
    pushHistory(message);

    printf("Road connected.\n");
}

void bfs(int start) {
    int visited[MAX_LOCATIONS] = {0};
    int queue[MAX_LOCATIONS], head = 0, tail = 0;

    queue[tail++] = start;
    visited[start] = 1;

    printf("BFS: ");
    while (head < tail) {
        int v = queue[head++];
        printf("%s -> ", locations[v]);

        for (int i = 0; i < locationCount; i++) {
            if (graph[v][i] && !visited[i]) {
                visited[i] = 1;
                queue[tail++] = i;
            }
        }
    }
    printf("END\n");
}

void dfsVisit(int v, int visited[]) {
    visited[v] = 1;
    printf("%s -> ", locations[v]);

    for (int i = 0; i < locationCount; i++)
        if (graph[v][i] && !visited[i])
            dfsVisit(i, visited);
}

void runTraversal(void) {
    char name[MAX_TEXT];
    printf("Starting location: ");
    scanf(" %99[^\n]", name);

    int start = findLocation(name);
    if (start < 0) {
        printf("Location not found.\n");
        return;
    }

    int choice;
    printf("1. BFS\n2. DFS\nChoice: ");
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return;
    }

    if (choice == 1) bfs(start);
    else if (choice == 2) {
        int visited[MAX_LOCATIONS] = {0};
        printf("DFS: ");
        dfsVisit(start, visited);
        printf("END\n");
    } else {
        printf("Invalid choice.\n");
    }
}

/* ---------- MAIN MENU ---------- */

void createDataDirectory(void) {
    /* On Windows, mkdir needs <direct.h>.
       On Linux/macOS, mkdir needs <sys/stat.h>. */
#ifdef _WIN32
    system("if not exist data mkdir data");
#else
    system("mkdir -p data");
#endif
}

void processNextRequest(void) {
    if (!front) {
        printf("No pending requests.\n");
        return;
    }

    Request *p = front;

    printf("Processing request #%d for %s at %s\n",
           p->id, p->name, p->location);

    char message[MAX_TEXT];
    snprintf(message, sizeof(message), "Processed request #%d",
             p->id);

    /* Record before removing, so a history write can be attempted. */
    pushHistory(message);

    front = front->next;
    if (!front) rear = NULL;

    free(p);
    saveRequests();

    printf("Request removed from pending queue.\n");
}

void cleanup(void) {
    while (front) {
        Request *p = front;
        front = front->next;
        free(p);
    }

    while (resourceHead) {
        Resource *p = resourceHead;
        resourceHead = resourceHead->next;
        free(p);
    }
}

int main(void) {
    createDataDirectory();
    loadRequests();
    loadResources();
    loadGraph();
    loadHistory();

    int choice;

    do {
        printf("\n====================================\n");
        printf(" DISASTER RESCUE & RESOURCE SYSTEM\n");
        printf("====================================\n");
        printf("1. Add rescue request\n");
        printf("2. View pending requests\n");
        printf("3. Process next request\n");
        printf("4. Add/replenish resource\n");
        printf("5. View resource inventory\n");
        printf("6. Allocate resource\n");
        printf("7. Add location\n");
        printf("8. Add road\n");
        printf("9. BFS/DFS traversal\n");
        printf("10. View operation history\n");
        printf("0. Save and exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Please enter a valid number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: addRequest(); break;
            case 2: displayRequests(); break;
            case 3: processNextRequest(); break;
            case 4: addResource(); break;
            case 5: displayResources(); break;
            case 6: allocateResource(); break;
            case 7: addLocation(); break;
            case 8: addRoad(); break;
            case 9: runTraversal(); break;
            case 10: displayHistory(); break;
            case 0:
                saveRequests();
                saveResources();
                saveGraph();
                printf("Data saved. Goodbye!\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 0);

    cleanup();
    return 0;
}
