/**
 * API Client Module for Disaster Rescue & Management System
 * C++17 REST Backend Interface
 */

const API = {
    baseUrl: window.location.origin,

    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const defaultHeaders = {
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        };

        const config = {
            ...options,
            headers: {
                ...defaultHeaders,
                ...options.headers
            }
        };

        try {
            const response = await fetch(url, config);
            const contentType = response.headers.get('content-type') || '';
            let data = null;

            if (contentType.includes('application/json')) {
                data = await response.json();
            } else {
                data = await response.text();
            }

            if (!response.ok) {
                const errorMessage = (data && data.message) ? data.message : `HTTP Error ${response.status}: ${response.statusText}`;
                throw new Error(errorMessage);
            }

            return data;
        } catch (error) {
            console.error(`API Error on ${endpoint}:`, error);
            throw error;
        }
    },

    // 1. Health & Status
    async checkHealth() {
        return this.request('/api/health');
    },

    // 2. Dashboard Statistics
    async getStats() {
        return this.request('/api/stats');
    },

    // 3. Disaster Reports (Singly Linked List)
    async getReports(params = {}) {
        const queryParams = new URLSearchParams();
        if (params.status && params.status !== 'all') queryParams.append('status', params.status);
        if (params.severity && params.severity !== 'all') queryParams.append('severity', params.severity);
        if (params.search) queryParams.append('search', params.search);
        if (params.sortBy) queryParams.append('sortBy', params.sortBy);

        const qs = queryParams.toString() ? `?${queryParams.toString()}` : '';
        return this.request(`/api/reports${qs}`);
    },

    async createReport(payload) {
        return this.request('/api/reports', {
            method: 'POST',
            body: JSON.stringify(payload)
        });
    },

    async escalateReport(id) {
        return this.request('/api/reports/escalate', {
            method: 'POST',
            body: JSON.stringify({ id })
        });
    },

    // 4. Emergency Prioritization (Binary Max-Heap Priority Queue)
    async getPriorityQueue() {
        return this.request('/api/priority-queue');
    },

    async dispatchTriageNext() {
        return this.request('/api/priority-queue/triage-next', {
            method: 'POST'
        });
    },

    async dispatchFifoNext() {
        return this.request('/api/priority-queue/fifo-next', {
            method: 'POST'
        });
    },

    // 5. Rescue Teams (Linked List)
    async getTeams() {
        return this.request('/api/teams');
    },

    async createTeam(payload) {
        return this.request('/api/teams', {
            method: 'POST',
            body: JSON.stringify(payload)
        });
    },

    async releaseTeam(teamId, newLocation = 'Relief Camp') {
        return this.request('/api/teams/release', {
            method: 'POST',
            body: JSON.stringify({ teamId, newLocation })
        });
    },

    // 6. Rescue Assignment Workflow (Module 8)
    async getAssignmentRecommendation(reportId) {
        return this.request(`/api/assignment/recommend?reportId=${encodeURIComponent(reportId)}`);
    },

    async confirmAssignment(reportId, teamId) {
        return this.request('/api/assignment/confirm', {
            method: 'POST',
            body: JSON.stringify({ reportId, teamId })
        });
    },

    async updateAssignmentStatus(reportId, status) {
        return this.request('/api/assignment/update-status', {
            method: 'POST',
            body: JSON.stringify({ reportId, status })
        });
    },

    // 7. Resources & Inventory (Linked List)
    async getResources() {
        return this.request('/api/resources');
    },

    async createResource(payload) {
        return this.request('/api/resources', {
            method: 'POST',
            body: JSON.stringify(payload)
        });
    },

    async allocateResource(payload) {
        return this.request('/api/resources/allocate', {
            method: 'POST',
            body: JSON.stringify(payload)
        });
    },

    async getAllocations() {
        return this.request('/api/allocations');
    },

    // 8. Locations & Roads Network (Adjacency List Graph)
    async getLocations() {
        return this.request('/api/locations');
    },

    async createLocation(payload) {
        return this.request('/api/locations', {
            method: 'POST',
            body: JSON.stringify(payload)
        });
    },

    async getRoads() {
        return this.request('/api/roads');
    },

    async createRoad(payload) {
        return this.request('/api/roads', {
            method: 'POST',
            body: JSON.stringify(payload)
        });
    },

    async toggleRoadBlock(roadId) {
        return this.request('/api/roads/toggle-block', {
            method: 'POST',
            body: JSON.stringify({ roadId })
        });
    },

    // 9. Route Exploration & Dijkstra Shortest Path
    async runDijkstra(start, destination) {
        return this.request(`/api/routes/dijkstra?start=${encodeURIComponent(start)}&destination=${encodeURIComponent(destination)}`);
    },

    async runBFS(startLocation) {
        return this.request(`/api/routes/bfs?start=${encodeURIComponent(startLocation)}`);
    },

    async runDFS(startLocation) {
        return this.request(`/api/routes/dfs?start=${encodeURIComponent(startLocation)}`);
    },

    // 10. Operations History (Stack)
    async getHistory(mode = 'recent') {
        return this.request(`/api/history?mode=${encodeURIComponent(mode)}`);
    },

    // 11. Persistence, Demo Data, Export
    async saveData() {
        return this.request('/api/save', {
            method: 'POST'
        });
    },

    async resetDemoData() {
        return this.request('/api/demo', {
            method: 'POST'
        });
    },

    async exportData() {
        return this.request('/api/export');
    }
};

window.API = API;
