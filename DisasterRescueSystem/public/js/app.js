/**
 * Disaster Rescue & Resource System - Frontend Application Controller
 * Handles REST API communication with the C++ backend and interactive DS visualizations
 */

const API_BASE = ''; // Same origin

let graphCanvas = null;
let currentLocations = [];
let currentRoads = [];
let selectedAlgorithm = 'bfs';
let isTraversing = false;

document.addEventListener('DOMContentLoaded', () => {
    graphCanvas = new GraphCanvas('graph-canvas');

    setupEventListeners();
    refreshAllData();

    // Auto refresh every 15 seconds
    setInterval(refreshStatsOnly, 15000);
});

/* =========================================================================
 * 1. Event Listeners Setup
 * ========================================================================= */
function setupEventListeners() {
    // Refresh & Seed Demo Buttons
    document.getElementById('btn-refresh').addEventListener('click', () => {
        refreshAllData();
        showToast('System data refreshed from C++ backend', 'success');
    });

    document.getElementById('btn-seed-demo').addEventListener('click', async () => {
        try {
            const res = await fetch(`${API_BASE}/api/reset-demo`, { method: 'POST' });
            const data = await res.json();
            if (data.success) {
                showToast(data.message, 'success');
                refreshAllData();
            }
        } catch (err) {
            showToast('Failed to load demo scenario: ' + err.message, 'danger');
        }
    });

    // Enqueue Form
    document.getElementById('form-enqueue').addEventListener('submit', async (e) => {
        e.preventDefault();
        const name = document.getElementById('req-name').value.trim();
        const location = document.getElementById('req-location').value;
        const emergency = document.getElementById('req-emergency').value.trim();
        const people = parseInt(document.getElementById('req-people').value, 10);

        if (!name || !location || !emergency || people <= 0) {
            showToast('Please fill out all fields with valid values', 'warning');
            return;
        }

        try {
            const res = await fetch(`${API_BASE}/api/requests`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name, location, emergency, people })
            });
            const data = await res.json();
            if (data.success) {
                showToast(`Request #${data.id} enqueued into rescue queue!`, 'success');
                document.getElementById('form-enqueue').reset();
                refreshRequests();
                refreshHistory();
                refreshStatsOnly();
            } else {
                showToast(data.message, 'danger');
            }
        } catch (err) {
            showToast('Error connecting to C++ backend: ' + err.message, 'danger');
        }
    });

    // Dequeue Button
    document.getElementById('btn-process-request').addEventListener('click', async () => {
        try {
            const res = await fetch(`${API_BASE}/api/requests/process`, { method: 'POST' });
            const data = await res.json();
            if (data.success) {
                showToast(`Processed Request #${data.processed.id} (${data.processed.name}) at ${data.processed.location}`, 'success');
                refreshRequests();
                refreshHistory();
                refreshStatsOnly();
            } else {
                showToast(data.message, 'warning');
            }
        } catch (err) {
            showToast('Error processing request: ' + err.message, 'danger');
        }
    });

    // Resource Sub Tabs
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.sub-tab-pane').forEach(p => p.classList.remove('active'));
            btn.classList.add('active');
            document.getElementById(btn.dataset.tab).classList.add('active');
        });
    });

    // Add Resource Form
    document.getElementById('form-add-resource').addEventListener('submit', async (e) => {
        e.preventDefault();
        const name = document.getElementById('res-add-name').value.trim();
        const quantity = parseInt(document.getElementById('res-add-qty').value, 10);

        try {
            const res = await fetch(`${API_BASE}/api/resources`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name, quantity })
            });
            const data = await res.json();
            if (data.success) {
                showToast(`Supplies updated: +${quantity} units of ${name}`, 'success');
                document.getElementById('form-add-resource').reset();
                refreshResources();
                refreshHistory();
                refreshStatsOnly();
            } else {
                showToast(data.message, 'danger');
            }
        } catch (err) {
            showToast('Error updating resources: ' + err.message, 'danger');
        }
    });

    // Allocate Resource Form
    document.getElementById('form-alloc-resource').addEventListener('submit', async (e) => {
        e.preventDefault();
        const name = document.getElementById('res-alloc-name').value;
        const quantity = parseInt(document.getElementById('res-alloc-qty').value, 10);

        try {
            const res = await fetch(`${API_BASE}/api/resources/allocate`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name, quantity })
            });
            const data = await res.json();
            if (data.success) {
                showToast(`Dispatched ${quantity} units of ${name}. Remaining: ${data.remaining}`, 'success');
                refreshResources();
                refreshHistory();
                refreshStatsOnly();
            } else {
                showToast(data.message, 'danger');
            }
        } catch (err) {
            showToast('Error allocating resource: ' + err.message, 'danger');
        }
    });

    // Algorithm Selector Pills
    document.getElementById('btn-select-bfs').addEventListener('click', () => {
        selectedAlgorithm = 'bfs';
        document.getElementById('btn-select-bfs').classList.add('active');
        document.getElementById('btn-select-dfs').classList.remove('active');
        document.getElementById('inspect-ds-title').innerText = 'Underlying Queue (FIFO):';
    });

    document.getElementById('btn-select-dfs').addEventListener('click', () => {
        selectedAlgorithm = 'dfs';
        document.getElementById('btn-select-dfs').classList.add('active');
        document.getElementById('btn-select-bfs').classList.remove('active');
        document.getElementById('inspect-ds-title').innerText = 'Underlying Call Stack (LIFO):';
    });

    // Run Traversal Button
    document.getElementById('btn-run-traversal').addEventListener('click', runTraversal);

    // Add Location Form
    document.getElementById('form-add-location').addEventListener('submit', async (e) => {
        e.preventDefault();
        const name = document.getElementById('new-loc-name').value.trim();
        if (!name) return;

        try {
            const res = await fetch(`${API_BASE}/api/locations`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name })
            });
            const data = await res.json();
            if (data.success) {
                showToast(`Location "${name}" added to graph!`, 'success');
                document.getElementById('new-loc-name').value = '';
                refreshLocations();
                refreshHistory();
                refreshStatsOnly();
            } else {
                showToast(data.message, 'danger');
            }
        } catch (err) {
            showToast('Error adding location: ' + err.message, 'danger');
        }
    });

    // Add Road Form
    document.getElementById('form-add-road').addEventListener('submit', async (e) => {
        e.preventDefault();
        const first = document.getElementById('road-from').value;
        const second = document.getElementById('road-to').value;

        if (!first || !second || first === second) {
            showToast('Please select two distinct locations to connect', 'warning');
            return;
        }

        try {
            const res = await fetch(`${API_BASE}/api/roads`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ first, second })
            });
            const data = await res.json();
            if (data.success) {
                showToast(`Road connected: ${first} <-> ${second}`, 'success');
                refreshLocations();
                refreshHistory();
                refreshStatsOnly();
            } else {
                showToast(data.message, 'danger');
            }
        } catch (err) {
            showToast('Error connecting road: ' + err.message, 'danger');
        }
    });

    // Modals
    document.getElementById('btn-toggle-matrix').addEventListener('click', showMatrixModal);
    document.getElementById('btn-close-matrix').addEventListener('click', () => {
        document.getElementById('matrix-modal').classList.remove('open');
    });

    document.getElementById('btn-theory-modal').addEventListener('click', () => {
        document.getElementById('theory-modal').classList.add('open');
    });
    document.getElementById('btn-close-theory').addEventListener('click', () => {
        document.getElementById('theory-modal').classList.remove('open');
    });

    // Close modal on outside click
    window.addEventListener('click', (e) => {
        if (e.target.classList.contains('modal-backdrop')) {
            e.target.classList.remove('open');
        }
    });
}

/* =========================================================================
 * 2. Data Fetching & Rendering
 * ========================================================================= */
async function refreshAllData() {
    await Promise.all([
        refreshStatsOnly(),
        refreshRequests(),
        refreshResources(),
        refreshLocations(),
        refreshHistory()
    ]);
}

async function refreshStatsOnly() {
    try {
        const res = await fetch(`${API_BASE}/api/status`);
        if (!res.ok) return;
        const stats = await res.json();

        document.getElementById('stat-queue-count').innerText = stats.requestsCount;
        document.getElementById('stat-people-count').innerText = `${stats.totalPeopleInDistress} people`;
        document.getElementById('stat-resources-count').innerText = stats.resourcesCount;
        document.getElementById('stat-stock-count').innerText = `${stats.totalResourceUnits} units`;
        document.getElementById('stat-locations-count').innerText = stats.locationsCount;
        document.getElementById('stat-roads-count').innerText = `${stats.roadsCount} roads`;
        document.getElementById('stat-history-count').innerText = stats.historyCount;
    } catch (e) {
        console.error('Failed to update stats:', e);
    }
}

// Render Queue Visualizer
async function refreshRequests() {
    try {
        const res = await fetch(`${API_BASE}/api/requests`);
        const data = await res.json();
        const container = document.getElementById('visual-queue-track');
        container.innerHTML = '';

        const requests = data.requests || [];
        const count = requests.length;

        document.getElementById('queue-size-val').innerText = count;
        document.getElementById('queue-front-val').innerText = count > 0 ? `#${requests[0].id}` : 'NULL';
        document.getElementById('queue-rear-val').innerText = count > 0 ? `#${requests[count - 1].id}` : 'NULL';

        if (count === 0) {
            container.innerHTML = '<div class="empty-state">No pending requests in the rescue queue.</div>';
            return;
        }

        requests.forEach((req, idx) => {
            const isFront = (idx === 0);
            const isRear = (idx === count - 1);

            const wrapper = document.createElement('div');
            wrapper.className = 'queue-node-wrapper';

            let pointerTags = '';
            if (isFront) pointerTags += '<span class="node-pointer-tag pointer-tag-front">FRONT</span>';
            if (isRear) pointerTags += '<span class="node-pointer-tag pointer-tag-rear">REAR</span>';

            wrapper.innerHTML = `
                <div class="queue-node ${isFront ? 'node-front' : ''} ${isRear ? 'node-rear' : ''}">
                    ${pointerTags}
                    <div class="node-id">REQ #${req.id}</div>
                    <div class="node-title">${escapeHtml(req.name)}</div>
                    <div class="node-sub">
                        <span>📍 ${escapeHtml(req.location)}</span>
                        <span>👥 ${req.people}</span>
                    </div>
                    <div style="font-size:0.7rem; color:#f87171; margin-top:3px;">⚠️ ${escapeHtml(req.emergency)}</div>
                </div>
                ${!isRear ? '<div class="node-arrow">──►</div>' : ''}
            `;
            container.appendChild(wrapper);
        });
    } catch (e) {
        console.error('Error fetching requests:', e);
    }
}

// Render Linked List Visualizer
async function refreshResources() {
    try {
        const res = await fetch(`${API_BASE}/api/resources`);
        const data = await res.json();
        const container = document.getElementById('visual-linked-list-track');
        container.innerHTML = '';

        const resources = data.resources || [];
        const count = resources.length;

        document.getElementById('list-count-val').innerText = count;
        document.getElementById('list-head-val').innerText = count > 0 ? resources[0].name : 'NULL';

        // Update Alloc Dropdown
        const allocSelect = document.getElementById('res-alloc-name');
        allocSelect.innerHTML = '<option value="">Select Resource</option>';
        resources.forEach(r => {
            const opt = document.createElement('option');
            opt.value = r.name;
            opt.innerText = `${r.name} (${r.quantity} available)`;
            allocSelect.appendChild(opt);
        });

        if (count === 0) {
            container.innerHTML = '<div class="empty-state">No resources in inventory.</div>';
            return;
        }

        resources.forEach((item, idx) => {
            const isHead = (idx === 0);
            const wrapper = document.createElement('div');
            wrapper.className = 'list-node-wrapper';

            wrapper.innerHTML = `
                <div class="list-node ${isHead ? 'is-head' : ''}">
                    ${isHead ? '<span class="pointer-tag-head">HEAD</span>' : ''}
                    <div class="node-data-box">
                        <div class="node-data-name">${escapeHtml(item.name)}</div>
                        <div class="node-data-qty">${item.quantity} units</div>
                    </div>
                    <div class="node-next-box">next*</div>
                </div>
                <div class="node-arrow">──►</div>
            `;
            container.appendChild(wrapper);
        });

        const nullBox = document.createElement('div');
        nullBox.className = 'null-terminator';
        nullBox.innerText = 'NULL';
        container.appendChild(nullBox);

    } catch (e) {
        console.error('Error fetching resources:', e);
    }
}

// Render Locations & Roads on Graph Canvas
async function refreshLocations() {
    try {
        const res = await fetch(`${API_BASE}/api/locations`);
        const data = await res.json();
        currentLocations = data.locations || [];
        currentRoads = data.roads || [];

        // Update dropdowns
        updateLocationDropdowns(currentLocations);

        // Update canvas
        graphCanvas.setGraph(currentLocations, currentRoads);
    } catch (e) {
        console.error('Error fetching locations:', e);
    }
}

function updateLocationDropdowns(locations) {
    const selects = [
        document.getElementById('req-location'),
        document.getElementById('traverse-start-select'),
        document.getElementById('road-from'),
        document.getElementById('road-to')
    ];

    selects.forEach(sel => {
        if (!sel) return;
        const currentVal = sel.value;
        const firstOptionText = sel.id === 'traverse-start-select' ? 'Select Start Location' : 'Select Location';
        sel.innerHTML = `<option value="">${firstOptionText}</option>`;
        locations.forEach(loc => {
            const opt = document.createElement('option');
            opt.value = loc;
            opt.innerText = loc;
            sel.appendChild(opt);
        });
        if (locations.includes(currentVal)) {
            sel.value = currentVal;
        } else if (sel.id === 'traverse-start-select' && locations.length > 0) {
            sel.value = locations[0];
        }
    });
}

// Render Operation History (Stack Visualizer)
async function refreshHistory() {
    try {
        const res = await fetch(`${API_BASE}/api/history`);
        const data = await res.json();
        const container = document.getElementById('visual-stack-track');
        container.innerHTML = '';

        const historyList = data.history || [];
        if (historyList.length === 0) {
            container.innerHTML = '<div class="empty-state">No operations recorded yet.</div>';
            return;
        }

        historyList.forEach((msg, idx) => {
            const isTop = (idx === 0);
            const item = document.createElement('div');
            item.className = `stack-item ${isTop ? 'is-top' : ''}`;
            item.innerHTML = `
                <span>${escapeHtml(msg)}</span>
                ${isTop ? '<span class="stack-item-tag">TOP [LIFO]</span>' : `<span style="font-size:0.7rem; color:#64748b;">#${historyList.length - idx}</span>`}
            `;
            container.appendChild(item);
        });
    } catch (e) {
        console.error('Error fetching history:', e);
    }
}

/* =========================================================================
 * 3. Traversal Execution & Step-by-Step Animation
 * ========================================================================= */
async function runTraversal() {
    if (isTraversing) return;

    const startLoc = document.getElementById('traverse-start-select').value;
    if (!startLoc) {
        showToast('Please select a starting location for traversal', 'warning');
        return;
    }

    const btn = document.getElementById('btn-run-traversal');
    btn.disabled = true;
    isTraversing = true;
    graphCanvas.clearHighlights();

    try {
        const res = await fetch(`${API_BASE}/api/traversal?start=${encodeURIComponent(startLoc)}&type=${selectedAlgorithm}`);
        const data = await res.json();

        if (!data.steps || data.steps.length === 0) {
            showToast('No reachable locations from ' + startLoc, 'warning');
            btn.disabled = false;
            isTraversing = false;
            return;
        }

        const steps = data.steps;
        const speed = parseInt(document.getElementById('traversal-speed').value, 10);
        document.getElementById('inspect-title').innerText = `Running ${data.type} from "${data.startNode}"`;

        let stepIndex = 0;

        function playStep() {
            if (stepIndex >= steps.length) {
                document.getElementById('inspect-title').innerText = `Traversal Complete (${data.type})`;
                document.getElementById('inspect-explanation').innerHTML = `
                    🎉 <strong>Exploration Complete:</strong> Successfully visited ${data.order.length} location(s) in order:
                    <br><code>${data.order.join(' ➔ ')}</code>
                `;
                btn.disabled = false;
                isTraversing = false;
                refreshHistory();
                refreshStatsOnly();
                return;
            }

            const step = steps[stepIndex];
            document.getElementById('inspect-step-counter').innerText = `Step ${stepIndex + 1} of ${steps.length}`;

            // 1. Highlight canvas
            graphCanvas.highlightStep(step.current, step.fromNode, step.visitedSoFar);

            // 2. Update order sequence badges
            const orderContainer = document.getElementById('inspect-order-badges');
            orderContainer.innerHTML = '';
            step.visitedSoFar.forEach(item => {
                const b = document.createElement('span');
                b.className = `badge-item ${item === step.current ? 'active-item' : ''}`;
                b.innerText = item;
                orderContainer.appendChild(b);
            });

            // 3. Update DS State badges (Queue or Stack)
            const dsContainer = document.getElementById('inspect-ds-badges');
            dsContainer.innerHTML = '';
            if (step.dsState && step.dsState.length > 0) {
                step.dsState.forEach(item => {
                    const b = document.createElement('span');
                    b.className = `badge-item ${item === step.current ? 'active-item' : ''}`;
                    b.innerText = item;
                    dsContainer.appendChild(b);
                });
            } else {
                dsContainer.innerHTML = '<span class="badge-item">[ Empty ]</span>';
            }

            // 4. Update Explanation
            document.getElementById('inspect-explanation').innerText = step.explanation;

            stepIndex++;
            setTimeout(playStep, speed);
        }

        playStep();

    } catch (err) {
        showToast('Traversal error: ' + err.message, 'danger');
        btn.disabled = false;
        isTraversing = false;
    }
}

/* =========================================================================
 * 4. Adjacency Matrix Modal Builder
 * ========================================================================= */
async function showMatrixModal() {
    try {
        const res = await fetch(`${API_BASE}/api/locations`);
        const data = await res.json();
        const locations = data.locations || [];
        const matrix = data.matrix || [];

        const container = document.getElementById('matrix-table-container');
        if (locations.length === 0) {
            container.innerHTML = '<div class="empty-state">No locations registered in the graph.</div>';
        } else {
            let html = '<table class="matrix-table"><thead><tr><th>LOC</th>';
            locations.forEach((loc, i) => {
                html += `<th title="${escapeHtml(loc)}">[${i}] ${escapeHtml(loc.slice(0, 8))}</th>`;
            });
            html += '</tr></thead><tbody>';

            matrix.forEach((row, i) => {
                html += `<tr><th>[${i}] ${escapeHtml(locations[i])}</th>`;
                row.forEach((val) => {
                    html += `<td class="cell-${val}">${val}</td>`;
                });
                html += '</tr>';
            });

            html += '</tbody></table>';
            container.innerHTML = html;
        }

        document.getElementById('matrix-modal').classList.add('open');
    } catch (e) {
        showToast('Error loading adjacency matrix: ' + e.message, 'danger');
    }
}

/* =========================================================================
 * 5. Utility Helpers & Toast Manager
 * ========================================================================= */
function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.innerText = message;
    container.appendChild(toast);

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateY(10px)';
        toast.style.transition = 'all 0.3s ease';
        setTimeout(() => toast.remove(), 300);
    }, 3500);
}

function escapeHtml(str) {
    if (!str) return '';
    return str.replace(/[&<>"']/g, function (m) {
        switch (m) {
            case '&': return '&amp;';
            case '<': return '&lt;';
            case '>': return '&gt;';
            case '"': return '&quot;';
            case "'": return '&#039;';
            default: return m;
        }
    });
}
