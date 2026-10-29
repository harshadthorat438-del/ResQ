/**
 * Main Application Controller for ResQ — Disaster Response & Relief Network
 * Full-Stack C++17 REST Backend Interface • Humanitarian Portal & Operational Dashboard
 */

document.addEventListener('DOMContentLoaded', () => {
    // -------------------------------------------------------------
    // Application State Store
    // -------------------------------------------------------------
    const state = {
        currentView: 'homepage', // 'homepage' | 'dashboard'
        currentSection: 'section-dashboard',
        stats: null,
        reports: [],
        prioritizedReports: [],
        teams: [],
        resources: [],
        allocations: [],
        locations: [],
        roads: [],
        history: [],
        selectedRecommendation: null
    };

    // -------------------------------------------------------------
    // Initialize SVG Graph Visualizers
    // -------------------------------------------------------------
    const graphVisualizer = new DisasterGraphVisualizer('network-svg-canvas');
    const previewVisualizer = new DisasterGraphVisualizer('network-svg-preview');

    graphVisualizer.onNodeClick((nodeName) => {
        const startSel = document.getElementById('route-start-select');
        const destSel = document.getElementById('route-dest-select');
        if (startSel && !startSel.value) {
            startSel.value = nodeName;
            showToast(`Origin set to: ${nodeName}`, 'info');
        } else if (destSel && destSel.value !== nodeName) {
            destSel.value = nodeName;
            showToast(`Destination set to: ${nodeName}`, 'info');
        } else if (startSel) {
            startSel.value = nodeName;
            showToast(`Origin changed to: ${nodeName}`, 'info');
        }
    });

    // -------------------------------------------------------------
    // 1. Toast Notification System
    // -------------------------------------------------------------
    function showToast(message, type = 'info', duration = 3500) {
        const container = document.getElementById('toast-container');
        if (!container) return;

        const icons = {
            success: '<svg class="resq-icon" style="color:var(--color-success-green)"><use href="#icon-check-circle"></use></svg>',
            error: '<svg class="resq-icon" style="color:var(--color-critical-red)"><use href="#icon-x-circle"></use></svg>',
            warning: '<svg class="resq-icon" style="color:var(--color-rescue-yellow)"><use href="#icon-alert-triangle"></use></svg>',
            info: '<svg class="resq-icon" style="color:var(--color-deep-blue)"><use href="#icon-activity"></use></svg>'
        };

        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        toast.innerHTML = `
            <span>${icons[type] || icons.info}</span>
            <div style="flex: 1;">${escapeHtml(message)}</div>
        `;

        container.appendChild(toast);

        setTimeout(() => {
            toast.style.opacity = '0';
            toast.style.transform = 'translateX(40px)';
            toast.style.transition = 'all 0.25s ease';
            setTimeout(() => toast.remove(), 250);
        }, duration);
    }

    function escapeHtml(str) {
        if (!str) return '';
        return String(str)
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#039;');
    }

    // -------------------------------------------------------------
    // 2. View Switching (Homepage vs Dashboard)
    // -------------------------------------------------------------
    function switchToView(viewName, targetSection = null) {
        const homeView = document.getElementById('view-homepage');
        const dashView = document.getElementById('view-dashboard');

        if (viewName === 'dashboard') {
            state.currentView = 'dashboard';
            if (homeView) homeView.classList.remove('active-view');
            if (dashView) dashView.classList.add('active-view');
            window.scrollTo({ top: 0, behavior: 'smooth' });

            if (targetSection) {
                switchSection(targetSection);
            } else {
                switchSection(state.currentSection || 'section-dashboard');
            }

            setTimeout(() => {
                graphVisualizer.computeLayout();
                graphVisualizer.render();
            }, 80);
        } else {
            state.currentView = 'homepage';
            if (dashView) dashView.classList.remove('active-view');
            if (homeView) homeView.classList.add('active-view');
            window.scrollTo({ top: 0, behavior: 'smooth' });

            setTimeout(() => {
                previewVisualizer.computeLayout();
                previewVisualizer.render();
            }, 80);
        }
    }

    // -------------------------------------------------------------
    // 3. Dashboard Section Navigation
    // -------------------------------------------------------------
    const menuItems = document.querySelectorAll('.dash-sidebar .menu-item');
    const dashSections = document.querySelectorAll('.dash-section');
    const sidebar = document.getElementById('dash-sidebar');
    const mobileDashToggle = document.getElementById('btn-toggle-dash-sidebar');
    const breadcrumbTitle = document.getElementById('current-section-title');

    function switchSection(targetSectionId) {
        menuItems.forEach(item => {
            if (item.dataset.section === targetSectionId) {
                item.classList.add('active');
                if (breadcrumbTitle) {
                    const span = item.querySelector('.menu-item-left span:last-child');
                    breadcrumbTitle.textContent = span ? span.textContent : 'Section';
                }
            } else {
                item.classList.remove('active');
            }
        });

        dashSections.forEach(section => {
            if (section.id === targetSectionId) {
                section.classList.add('active-section');
            } else {
                section.classList.remove('active-section');
            }
        });

        state.currentSection = targetSectionId;

        if (targetSectionId === 'section-routes') {
            setTimeout(() => {
                graphVisualizer.computeLayout();
                graphVisualizer.render();
            }, 50);
        }

        // Close sidebar on mobile
        if (window.innerWidth <= 768 && sidebar) {
            sidebar.classList.remove('open');
        }
    }

    menuItems.forEach(item => {
        item.addEventListener('click', () => {
            const target = item.dataset.section;
            if (target) switchSection(target);
        });
    });

    if (mobileDashToggle && sidebar) {
        mobileDashToggle.addEventListener('click', () => {
            sidebar.classList.toggle('open');
        });
    }

    // -------------------------------------------------------------
    // 4. Global Action Handlers & Event Listeners
    // -------------------------------------------------------------
    // Mobile Navbar Menu Toggle
    const mobileMenuBtn = document.getElementById('btn-mobile-menu');
    const navLinks = document.querySelector('.nav-links');
    if (mobileMenuBtn && navLinks) {
        mobileMenuBtn.addEventListener('click', () => {
            navLinks.classList.toggle('mobile-open');
        });
        navLinks.querySelectorAll('a').forEach(link => {
            link.addEventListener('click', () => {
                navLinks.classList.remove('mobile-open');
            });
        });
    }

    // View Switchers
    document.getElementById('btn-nav-open-dashboard')?.addEventListener('click', () => switchToView('dashboard'));
    document.getElementById('btn-hero-open-dash')?.addEventListener('click', () => switchToView('dashboard'));
    document.getElementById('btn-final-dashboard')?.addEventListener('click', () => switchToView('dashboard'));
    document.querySelectorAll('.footer-open-dash').forEach(el => {
        el.addEventListener('click', (e) => {
            e.preventDefault();
            switchToView('dashboard');
        });
    });

    document.getElementById('btn-return-home')?.addEventListener('click', () => switchToView('homepage'));
    document.getElementById('brand-home-link')?.addEventListener('click', () => switchToView('homepage'));

    // Hero Explore Network -> Smooth scroll to #network
    document.getElementById('btn-hero-explore-network')?.addEventListener('click', () => {
        const netEl = document.getElementById('network');
        if (netEl) netEl.scrollIntoView({ behavior: 'smooth' });
    });

    // Network Preview CTA -> Open Dashboard Routes
    document.getElementById('btn-preview-open-routes')?.addEventListener('click', () => {
        switchToView('dashboard', 'section-routes');
    });

    // Emergency Report Modal Triggers
    const reportModalTriggers = [
        document.getElementById('btn-home-report-emergency'),
        document.getElementById('btn-hero-report'),
        document.getElementById('btn-final-report'),
        document.getElementById('btn-dash-new-report'),
        document.getElementById('btn-open-report-modal')
    ];

    reportModalTriggers.forEach(btn => {
        btn?.addEventListener('click', () => {
            populateLocationDropdowns();
            openModal('modal-register-report');
        });
    });

    document.querySelectorAll('.footer-open-report').forEach(el => {
        el.addEventListener('click', (e) => {
            e.preventDefault();
            populateLocationDropdowns();
            openModal('modal-register-report');
        });
    });

    // Initiative Action Buttons
    document.querySelectorAll('.initiative-action-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const emergency = btn.dataset.emergency;
            const resource = btn.dataset.resource;
            const targetSec = btn.dataset.targetSection;

            if (emergency) {
                populateLocationDropdowns();
                const emSel = document.getElementById('report-emergency-type');
                const resSel = document.getElementById('report-resource-type');
                if (emSel) emSel.value = emergency;
                if (resSel && resource) resSel.value = resource;
                openModal('modal-register-report');
            } else if (targetSec) {
                switchToView('dashboard', targetSec);
            }
        });
    });

    // Quick Action Buttons in Dashboard Overview
    document.getElementById('btn-dash-quick-allocate')?.addEventListener('click', () => {
        populateAllocateDropdowns();
        openModal('modal-allocate-resource');
    });

    document.getElementById('btn-dash-view-teams')?.addEventListener('click', () => {
        switchSection('section-teams');
    });

    document.getElementById('btn-dash-view-inventory')?.addEventListener('click', () => {
        switchSection('section-resources');
    });

    // Other Dashboard Modals
    document.getElementById('btn-open-team-modal')?.addEventListener('click', () => {
        populateLocationDropdowns();
        openModal('modal-add-team');
    });

    document.getElementById('btn-open-resource-modal')?.addEventListener('click', () => {
        openModal('modal-add-resource');
    });

    document.getElementById('btn-open-allocate-modal')?.addEventListener('click', () => {
        populateAllocateDropdowns();
        openModal('modal-allocate-resource');
    });

    document.getElementById('btn-open-location-modal')?.addEventListener('click', () => {
        openModal('modal-add-location');
    });

    document.getElementById('btn-open-road-modal')?.addEventListener('click', () => {
        populateRoadLocationDropdowns();
        openModal('modal-add-road');
    });

    // -------------------------------------------------------------
    // 5. Modals Management Helpers
    // -------------------------------------------------------------
    function openModal(modalId) {
        const modal = document.getElementById(modalId);
        if (modal) modal.classList.add('active');
    }

    function closeModal(modalId) {
        const modal = document.getElementById(modalId);
        if (modal) modal.classList.remove('active');
    }

    document.querySelectorAll('[data-close-modal]').forEach(btn => {
        btn.addEventListener('click', () => {
            const modalId = btn.getAttribute('data-close-modal');
            if (modalId) closeModal(modalId);
        });
    });

    // Close on backdrop click
    document.querySelectorAll('.modal-overlay').forEach(overlay => {
        overlay.addEventListener('click', (e) => {
            if (e.target === overlay) {
                overlay.classList.remove('active');
            }
        });
    });

    // -------------------------------------------------------------
    // 6. Dropdown Population Helpers
    // -------------------------------------------------------------
    function populateLocationDropdowns() {
        const locSelects = [
            document.getElementById('report-location'),
            document.getElementById('team-station'),
            document.getElementById('route-start-select'),
            document.getElementById('route-dest-select')
        ];

        locSelects.forEach(sel => {
            if (!sel) return;
            const curVal = sel.value;
            sel.innerHTML = '<option value="">Select location...</option>';
            state.locations.forEach(loc => {
                const opt = document.createElement('option');
                opt.value = loc.name;
                opt.textContent = `${loc.name} (${loc.type})`;
                sel.appendChild(opt);
            });
            if (curVal && state.locations.some(l => l.name === curVal)) {
                sel.value = curVal;
            }
        });
    }

    function populateRoadLocationDropdowns() {
        const fromSel = document.getElementById('road-from');
        const toSel = document.getElementById('road-to');
        if (!fromSel || !toSel) return;

        fromSel.innerHTML = '<option value="">Select origin...</option>';
        toSel.innerHTML = '<option value="">Select destination...</option>';
        state.locations.forEach(loc => {
            const o1 = document.createElement('option');
            o1.value = loc.name;
            o1.textContent = loc.name;
            fromSel.appendChild(o1);

            const o2 = document.createElement('option');
            o2.value = loc.name;
            o2.textContent = loc.name;
            toSel.appendChild(o2);
        });

        if (state.locations.length >= 2) {
            fromSel.selectedIndex = 1;
            toSel.selectedIndex = 2;
        }
    }

    function populateAllocateDropdowns() {
        const reportSel = document.getElementById('alloc-report-id');
        const resSel = document.getElementById('alloc-resource-name');
        if (!reportSel || !resSel) return;

        reportSel.innerHTML = '<option value="">Select target report...</option>';
        state.reports.forEach(r => {
            if (r.status !== 'Completed' && r.status !== 'Cancelled') {
                const opt = document.createElement('option');
                opt.value = r.id;
                opt.textContent = `${r.id} - ${r.location} (${r.emergencyType}, ${r.priority})`;
                reportSel.appendChild(opt);
            }
        });

        resSel.innerHTML = '<option value="">Select resource...</option>';
        state.resources.forEach(res => {
            const opt = document.createElement('option');
            opt.value = res.name;
            opt.textContent = `${res.name} (Stock: ${res.quantity} ${res.unit})`;
            resSel.appendChild(opt);
        });
    }

    // -------------------------------------------------------------
    // 7. Data Synchronization (Fetch All C++ Endpoints)
    // -------------------------------------------------------------
    async function loadAllData() {
        try {
            // Health Check
            const health = await API.checkHealth();
            const pulse = document.getElementById('backend-status-pulse');
            const statusText = document.getElementById('backend-status-text');
            if (pulse && statusText) {
                pulse.style.backgroundColor = '#287A58';
                statusText.textContent = `Backend Online (${health.version || 'v2.5.0'})`;
            }

            // Fetch datasets concurrently
            const [
                statsRes,
                reportsRes,
                priorityRes,
                teamsRes,
                resourcesRes,
                allocationsRes,
                locationsRes,
                roadsRes,
                historyRes
            ] = await Promise.all([
                API.getStats(),
                API.getReports(),
                API.getPriorityQueue(),
                API.getTeams(),
                API.getResources(),
                API.getAllocations(),
                API.getLocations(),
                API.getRoads(),
                API.getHistory('all')
            ]);

            state.stats = statsRes;
            state.reports = reportsRes.reports || [];
            state.prioritizedReports = priorityRes.prioritizedReports || [];
            state.teams = teamsRes.teams || [];
            state.resources = resourcesRes.resources || [];
            state.allocations = allocationsRes.allocations || [];
            state.locations = locationsRes.locations || [];
            state.roads = roadsRes.roads || [];
            state.history = historyRes.operations || [];

            // Update UI components
            renderDashboard();
            renderReportsTable();
            renderPriorityTable();
            renderTeamsTable();
            renderResourcesTable();
            renderAllocationsTable();
            renderLocationsAndRoads();
            renderOperationsTable();
            renderHistoryTable();

            populateLocationDropdowns();

            // Render SVG Graphs (both Dashboard & Homepage Preview)
            graphVisualizer.setData(state.locations, state.roads);
            previewVisualizer.setData(state.locations, state.roads);

        } catch (err) {
            console.error('Data sync failed:', err);
            const pulse = document.getElementById('backend-status-pulse');
            const statusText = document.getElementById('backend-status-text');
            if (pulse && statusText) {
                pulse.style.backgroundColor = '#C83D3D';
                statusText.textContent = 'Backend Offline';
            }
        }
    }

    // -------------------------------------------------------------
    // 8. Render Dashboard & Homepage Metrics
    // -------------------------------------------------------------
    function renderDashboard() {
        if (!state.stats) return;
        const s = state.stats;

        // Dashboard Metric Widgets
        const elPending = document.getElementById('stat-pending-req');
        if (elPending) elPending.textContent = s.pendingRequests || 0;

        const elAffected = document.getElementById('stat-affected-people');
        if (elAffected) elAffected.textContent = `${s.totalAffectedPeople || 0} people waiting`;

        const elCritHigh = document.getElementById('stat-critical-high');
        if (elCritHigh) elCritHigh.textContent = s.criticalAndHighReports || 0;

        const elImmed = document.getElementById('stat-immediate-rescue');
        if (elImmed) elImmed.textContent = `${s.totalImmediateRescue || 0} need immediate rescue`;

        const elActiveOps = document.getElementById('stat-active-ops');
        if (elActiveOps) elActiveOps.textContent = s.activeOperations || 0;

        const elCompOps = document.getElementById('stat-completed-ops');
        if (elCompOps) elCompOps.textContent = `${s.completedOperations || 0} completed`;

        const elAvailTeams = document.getElementById('stat-available-teams');
        if (elAvailTeams) elAvailTeams.textContent = s.availableTeams || 0;

        const elBusyTeams = document.getElementById('stat-busy-teams');
        if (elBusyTeams) elBusyTeams.textContent = `${s.busyTeams || 0} busy on missions`;

        const elTotalRes = document.getElementById('stat-total-resources');
        if (elTotalRes) elTotalRes.textContent = s.totalResourceUnits || 0;

        const elLowStock = document.getElementById('stat-low-stock-count');
        if (elLowStock) elLowStock.textContent = `${s.lowStockCount || 0} items low in stock`;

        const elLocations = document.getElementById('stat-locations');
        if (elLocations) elLocations.textContent = s.locationCount || 0;

        const elRoads = document.getElementById('stat-roads-count');
        if (elRoads) elRoads.textContent = `${s.roadCount || 0} roads (${s.blockedRoadCount || 0} blocked)`;

        // Sidebar Badges
        const repBadge = document.getElementById('nav-reports-badge');
        if (repBadge) repBadge.textContent = s.pendingRequests || 0;

        const priBadge = document.getElementById('nav-priority-badge');
        if (priBadge) priBadge.textContent = s.criticalAndHighReports || 0;

        const opsBadge = document.getElementById('nav-ops-badge');
        if (opsBadge) opsBadge.textContent = s.activeOperations || 0;

        // Homepage Hero Floating Pill
        const pillActive = document.getElementById('pill-active-ops');
        if (pillActive) pillActive.textContent = s.activeOperations || 0;

        const pillComm = document.getElementById('pill-communities');
        if (pillComm) pillComm.textContent = s.locationCount || 0;

        const pillTeams = document.getElementById('pill-avail-teams');
        if (pillTeams) pillTeams.textContent = s.availableTeams || 0;

        const pillSupplies = document.getElementById('pill-supplies-ready');
        if (pillSupplies) pillSupplies.textContent = s.totalResourceUnits || 0;

        // Homepage Impact Ribbon Strip
        const stripPending = document.getElementById('strip-stat-pending');
        if (stripPending) stripPending.textContent = s.pendingRequests || 0;

        const stripTeams = document.getElementById('strip-stat-teams');
        if (stripTeams) stripTeams.textContent = s.availableTeams || 0;

        const stripRes = document.getElementById('strip-stat-resources');
        if (stripRes) stripRes.textContent = s.totalResourceUnits || 0;

        const stripLoc = document.getElementById('strip-stat-locations');
        if (stripLoc) stripLoc.textContent = s.locationCount || 0;

        // Homepage Impact Section
        const homeRep = document.getElementById('home-stat-reports');
        if (homeRep) homeRep.textContent = s.totalReports || 0;

        const homeResc = document.getElementById('home-stat-rescues');
        if (homeResc) homeResc.textContent = s.completedOperations || 0;

        const homeAct = document.getElementById('home-stat-active');
        if (homeAct) homeAct.textContent = s.activeOperations || 0;

        const homeTms = document.getElementById('home-stat-teams');
        if (homeTms) homeTms.textContent = s.availableTeams || 0;

        const homeUnits = document.getElementById('home-stat-units');
        if (homeUnits) homeUnits.textContent = s.totalResourceUnits || 0;

        // Overview Pending Incidents Table
        const pendingTbody = document.getElementById('dash-pending-tbody');
        if (pendingTbody) {
            const active = state.prioritizedReports.slice(0, 5);
            if (active.length === 0) {
                pendingTbody.innerHTML = '<tr><td colspan="6" class="empty-cell">No pending emergency reports. All areas clear!</td></tr>';
            } else {
                pendingTbody.innerHTML = active.map(item => `
                    <tr>
                        <td><strong>${escapeHtml(item.id)}</strong></td>
                        <td>${escapeHtml(item.location)}</td>
                        <td>${escapeHtml(item.emergencyType)}</td>
                        <td><span class="badge badge-${getSeverityBadgeClass(item.priority)}">${escapeHtml(item.priority)}</span></td>
                        <td><strong style="color: var(--color-deep-blue);">${item.urgencyScore.toFixed(0)}</strong></td>
                        <td>
                            <button class="btn btn-yellow btn-sm" onclick="window.appRecommendAssignment('${item.id}')">
                                Dispatch
                            </button>
                        </td>
                    </tr>
                `).join('');
            }
        }

        // Overview Teams Table
        const teamsTbody = document.getElementById('dash-teams-tbody');
        if (teamsTbody) {
            if (state.teams.length === 0) {
                teamsTbody.innerHTML = '<tr><td colspan="5" class="empty-cell">No rescue teams registered.</td></tr>';
            } else {
                teamsTbody.innerHTML = state.teams.map(t => `
                    <tr>
                        <td><strong>${escapeHtml(t.id)}</strong></td>
                        <td>${escapeHtml(t.name)}</td>
                        <td>${escapeHtml(t.type)}</td>
                        <td>${escapeHtml(t.currentLocation)}</td>
                        <td><span class="badge badge-${t.status === 'Available' ? 'available' : 'busy'}">${escapeHtml(t.status)}</span></td>
                    </tr>
                `).join('');
            }
        }

        // Overview Inventory Table
        const invTbody = document.getElementById('dash-inventory-tbody');
        if (invTbody) {
            if (state.resources.length === 0) {
                invTbody.innerHTML = '<tr><td colspan="7" class="empty-cell">No resources in stock.</td></tr>';
            } else {
                invTbody.innerHTML = state.resources.map(r => {
                    const isLow = r.quantity <= r.minThreshold;
                    return `
                        <tr>
                            <td><strong>${escapeHtml(r.id)}</strong></td>
                            <td>${escapeHtml(r.name)}</td>
                            <td>${escapeHtml(r.category)}</td>
                            <td><strong>${r.quantity}</strong> ${escapeHtml(r.unit)}</td>
                            <td>${r.minThreshold} ${escapeHtml(r.unit)}</td>
                            <td><span class="badge badge-${isLow ? 'critical' : 'available'}">${isLow ? 'LOW STOCK' : 'Optimal'}</span></td>
                            <td>
                                <button class="btn btn-secondary btn-sm" onclick="window.appQuickReplenish('${escapeHtml(r.name)}')">
                                    + Replenish
                                </button>
                            </td>
                        </tr>
                    `;
                }).join('');
            }
        }
    }

    function getSeverityBadgeClass(severity) {
        switch (severity) {
            case 'Critical': return 'critical';
            case 'High': return 'high';
            case 'Moderate': return 'moderate';
            case 'Low': return 'low';
            default: return 'moderate';
        }
    }

    function getStatusBadgeClass(status) {
        switch (status) {
            case 'Reported':
            case 'Pending': return 'pending';
            case 'Assigned': return 'assigned';
            case 'In Progress': return 'progress';
            case 'Completed': return 'completed';
            case 'Cancelled': return 'cancelled';
            default: return 'pending';
        }
    }

    // -------------------------------------------------------------
    // 9. Render Section 2: Disaster Reports
    // -------------------------------------------------------------
    function renderReportsTable() {
        const tbody = document.getElementById('reports-table-tbody');
        if (!tbody) return;

        const search = document.getElementById('filter-reports-search')?.value.toLowerCase() || '';
        const statusFilter = document.getElementById('filter-reports-status')?.value || 'all';
        const severityFilter = document.getElementById('filter-reports-severity')?.value || 'all';

        let filtered = state.reports.filter(r => {
            if (statusFilter !== 'all' && r.status !== statusFilter) return false;
            if (severityFilter !== 'all' && r.priority !== severityFilter) return false;
            if (search) {
                const match = (r.id && r.id.toLowerCase().includes(search)) ||
                              (r.personName && r.personName.toLowerCase().includes(search)) ||
                              (r.location && r.location.toLowerCase().includes(search)) ||
                              (r.emergencyType && r.emergencyType.toLowerCase().includes(search));
                if (!match) return false;
            }
            return true;
        });

        if (filtered.length === 0) {
            tbody.innerHTML = '<tr><td colspan="10" class="empty-cell">No matching disaster reports found.</td></tr>';
            return;
        }

        tbody.innerHTML = filtered.map(r => `
            <tr>
                <td><strong>${escapeHtml(r.id)}</strong></td>
                <td>${escapeHtml(r.personName)}</td>
                <td><strong>${escapeHtml(r.location)}</strong></td>
                <td>${escapeHtml(r.emergencyType)}</td>
                <td><span class="badge badge-${getSeverityBadgeClass(r.priority)}">${escapeHtml(r.priority)}</span></td>
                <td>${r.affectedPeople} affected <span style="color: var(--color-critical-red);">(${r.immediateRescueCount} rescue)</span></td>
                <td><small>${escapeHtml(r.requiredResourceType)} (${r.requiredResourceQty})</small></td>
                <td>${r.assignedTeamId ? `<strong>${escapeHtml(r.assignedTeamId)}</strong>` : '<span style="color: var(--color-muted-slate);">Unassigned</span>'}</td>
                <td><span class="badge badge-${getStatusBadgeClass(r.status)}">${escapeHtml(r.status)}</span></td>
                <td>
                    <div style="display: flex; gap: 6px; flex-wrap: wrap;">
                        ${(r.status !== 'Completed' && r.status !== 'Cancelled') ? `
                            <button class="btn btn-yellow btn-sm" onclick="window.appRecommendAssignment('${r.id}')" title="Find recommended team via Dijkstra">
                                Dispatch
                            </button>
                            ${!r.escalated ? `
                                <button class="btn btn-secondary btn-sm" onclick="window.appEscalateReport('${r.id}')" title="Escalate Urgency">
                                    ⬆️ Escalate
                                </button>
                            ` : ''}
                        ` : ''}
                    </div>
                </td>
            </tr>
        `).join('');
    }

    document.getElementById('filter-reports-search')?.addEventListener('input', renderReportsTable);
    document.getElementById('filter-reports-status')?.addEventListener('change', renderReportsTable);
    document.getElementById('filter-reports-severity')?.addEventListener('change', renderReportsTable);
    document.getElementById('sort-reports-select')?.addEventListener('change', async (e) => {
        const sortBy = e.target.value;
        const res = await API.getReports({ sortBy });
        state.reports = res.reports || [];
        renderReportsTable();
    });

    // -------------------------------------------------------------
    // 10. Render Section 3: Priority Queue
    // -------------------------------------------------------------
    function renderPriorityTable() {
        const tbody = document.getElementById('priority-queue-tbody');
        const badge = document.getElementById('badge-priority-count');
        if (!tbody) return;

        const list = state.prioritizedReports;
        if (badge) badge.textContent = `${list.length} in queue`;

        if (list.length === 0) {
            tbody.innerHTML = '<tr><td colspan="8" class="empty-cell">No pending emergencies in Priority Queue.</td></tr>';
            return;
        }

        tbody.innerHTML = list.map((item, idx) => `
            <tr>
                <td><strong style="color: var(--color-deep-blue);">#${item.priorityRank || (idx + 1)}</strong></td>
                <td><strong style="color: var(--color-critical-red); font-size: 1.05rem;">${item.urgencyScore.toFixed(1)}</strong></td>
                <td><strong>${escapeHtml(item.id)}</strong></td>
                <td>${escapeHtml(item.location)}</td>
                <td>${escapeHtml(item.emergencyType)}</td>
                <td><span class="badge badge-${getSeverityBadgeClass(item.priority)}">${escapeHtml(item.priority)}</span></td>
                <td><strong>${item.immediateRescueCount}</strong> immediate</td>
                <td>
                    <div style="display: flex; gap: 6px;">
                        <button class="btn btn-yellow btn-sm" onclick="window.appRecommendAssignment('${item.id}')">
                            Dispatch Team
                        </button>
                        ${!item.escalated ? `
                            <button class="btn btn-secondary btn-sm" onclick="window.appEscalateReport('${item.id}')">
                                Escalate
                            </button>
                        ` : '<span style="font-size: 0.78rem; color: var(--color-critical-red); font-weight: 700;">Escalated</span>'}
                    </div>
                </td>
            </tr>
        `).join('');
    }

    // Triage & FIFO Dispatch buttons
    const triageHandlers = [
        document.getElementById('btn-dash-triage-next'),
        document.getElementById('btn-priority-triage-next')
    ];

    triageHandlers.forEach(btn => {
        btn?.addEventListener('click', async () => {
            try {
                const res = await API.dispatchTriageNext();
                showToast(`Triaged highest-urgency emergency: #${res.dispatched.id} (${res.dispatched.location})`, 'success');
                await loadAllData();
            } catch (err) {
                showToast(err.message, 'error');
            }
        });
    });

    document.getElementById('btn-priority-fifo-next')?.addEventListener('click', async () => {
        try {
            const res = await API.dispatchFifoNext();
            showToast(`Dequeued next pending emergency: #${res.dispatched.id} (${res.dispatched.location}) in FIFO order`, 'info');
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // -------------------------------------------------------------
    // 11. Render Section 4: Rescue Teams
    // -------------------------------------------------------------
    function renderTeamsTable() {
        const tbody = document.getElementById('teams-fleet-tbody');
        const badge = document.getElementById('badge-total-teams');
        if (!tbody) return;

        if (badge) badge.textContent = `${state.teams.length} Teams`;

        if (state.teams.length === 0) {
            tbody.innerHTML = '<tr><td colspan="8" class="empty-cell">No rescue teams registered in fleet.</td></tr>';
            return;
        }

        tbody.innerHTML = state.teams.map(t => `
            <tr>
                <td><strong>${escapeHtml(t.id)}</strong></td>
                <td><strong>${escapeHtml(t.name)}</strong></td>
                <td><span class="badge badge-moderate">${escapeHtml(t.type)}</span></td>
                <td>${escapeHtml(t.currentLocation)}</td>
                <td>${t.capacity} responders</td>
                <td><small>${escapeHtml(t.equipment)}</small></td>
                <td><span class="badge badge-${t.status === 'Available' ? 'available' : 'busy'}">${escapeHtml(t.status)}</span></td>
                <td>
                    ${t.status === 'Busy' ? `
                        <button class="btn btn-secondary btn-sm" onclick="window.appReleaseTeam('${t.id}')">
                            Release to Base
                        </button>
                    ` : '<span style="font-size: 0.8rem; color: var(--color-success-green); font-weight: 700;">Ready for Call</span>'}
                </td>
            </tr>
        `).join('');
    }

    // -------------------------------------------------------------
    // 12. Render Section 5: Resource Inventory
    // -------------------------------------------------------------
    function renderResourcesTable() {
        const tbody = document.getElementById('resources-table-tbody');
        if (!tbody) return;

        if (state.resources.length === 0) {
            tbody.innerHTML = '<tr><td colspan="8" class="empty-cell">No relief supplies registered in inventory.</td></tr>';
            return;
        }

        tbody.innerHTML = state.resources.map(r => {
            const isLow = r.quantity <= r.minThreshold;
            return `
                <tr>
                    <td><strong>${escapeHtml(r.id)}</strong></td>
                    <td><strong>${escapeHtml(r.name)}</strong></td>
                    <td>${escapeHtml(r.category)}</td>
                    <td><strong>${r.quantity}</strong> ${escapeHtml(r.unit)}</td>
                    <td>${r.minThreshold} ${escapeHtml(r.unit)}</td>
                    <td>${escapeHtml(r.unit)}</td>
                    <td><span class="badge badge-${isLow ? 'critical' : 'available'}">${isLow ? 'LOW STOCK' : 'In Stock'}</span></td>
                    <td>
                        <button class="btn btn-secondary btn-sm" onclick="window.appQuickReplenish('${escapeHtml(r.name)}')">
                            + Replenish
                        </button>
                    </td>
                </tr>
            `;
        }).join('');
    }

    // -------------------------------------------------------------
    // 13. Render Section 6: Resource Allocations
    // -------------------------------------------------------------
    function renderAllocationsTable() {
        const tbody = document.getElementById('allocations-table-tbody');
        if (!tbody) return;

        if (state.allocations.length === 0) {
            tbody.innerHTML = '<tr><td colspan="7" class="empty-cell">No supplies allocated yet.</td></tr>';
            return;
        }

        tbody.innerHTML = state.allocations.slice().reverse().map(a => `
            <tr>
                <td><strong>${escapeHtml(a.id)}</strong></td>
                <td><strong>${escapeHtml(a.requestId)}</strong></td>
                <td>${escapeHtml(a.personName)} (${escapeHtml(a.location)})</td>
                <td>${escapeHtml(a.resourceName)}</td>
                <td><strong style="color: var(--color-deep-blue);">${a.quantity}</strong></td>
                <td><small>${escapeHtml(a.notes || 'Emergency distribution')}</small></td>
                <td><small>${escapeHtml(a.timestamp)}</small></td>
            </tr>
        `).join('');
    }

    // -------------------------------------------------------------
    // 14. Render Section 7: Locations & Roads
    // -------------------------------------------------------------
    function renderLocationsAndRoads() {
        const locTbody = document.getElementById('locations-table-tbody');
        const roadTbody = document.getElementById('roads-table-tbody');

        if (locTbody) {
            if (state.locations.length === 0) {
                locTbody.innerHTML = '<tr><td colspan="4" class="empty-cell">No locations registered.</td></tr>';
            } else {
                locTbody.innerHTML = state.locations.map(l => `
                    <tr>
                        <td><strong>${escapeHtml(l.id)}</strong></td>
                        <td><strong>${escapeHtml(l.name)}</strong></td>
                        <td><span class="badge badge-moderate">${escapeHtml(l.type)}</span></td>
                        <td><small>(${l.x}, ${l.y})</small></td>
                    </tr>
                `).join('');
            }
        }

        if (roadTbody) {
            if (state.roads.length === 0) {
                roadTbody.innerHTML = '<tr><td colspan="5" class="empty-cell">No roads registered.</td></tr>';
            } else {
                roadTbody.innerHTML = state.roads.map(rd => {
                    const isBlocked = rd.status === 'Blocked';
                    return `
                        <tr>
                            <td><strong>${escapeHtml(rd.id)}</strong></td>
                            <td><strong>${escapeHtml(rd.from)}</strong> ↔ <strong>${escapeHtml(rd.to)}</strong></td>
                            <td>${rd.distance} min</td>
                            <td><span class="badge badge-${isBlocked ? 'blocked' : 'open'}">${escapeHtml(rd.status)}</span></td>
                            <td>
                                <button class="btn btn-${isBlocked ? 'success' : 'danger'} btn-sm" onclick="window.appToggleRoadBlock('${rd.id}')">
                                    ${isBlocked ? 'Reopen Road' : 'Block Road'}
                                </button>
                            </td>
                        </tr>
                    `;
                }).join('');
            }
        }
    }

    // -------------------------------------------------------------
    // 15. Render Section 8: Route Exploration
    // -------------------------------------------------------------
    document.getElementById('btn-run-dijkstra')?.addEventListener('click', async () => {
        const start = document.getElementById('route-start-select')?.value;
        const dest = document.getElementById('route-dest-select')?.value;

        if (!start || !dest) {
            showToast('Please select both Origin and Destination locations.', 'warning');
            return;
        }

        try {
            const data = await API.runDijkstra(start, dest);
            const res = data.result;

            const card = document.getElementById('route-result-card');
            const title = document.getElementById('route-result-title');
            const body = document.getElementById('route-result-body');

            if (card && body) {
                card.classList.add('active');
                title.innerHTML = `Shortest Route: <strong>${escapeHtml(start)} → ${escapeHtml(dest)}</strong>`;

                if (!res.isReachable) {
                    body.innerHTML = `
                        <div style="background: var(--color-critical-red-light); border: 1px solid var(--color-critical-red); padding: 14px; border-radius: var(--radius-sm); color: var(--color-critical-red);">
                            <h4 style="margin: 0 0 6px 0; font-weight: 700; display: flex; align-items: center; gap: 8px;"><svg class="resq-icon"><use href="#icon-alert-triangle"></use></svg> Destination Unreachable</h4>
                            <p style="margin: 0; font-size: 0.88rem;">No open route exists between <strong>${escapeHtml(start)}</strong> and <strong>${escapeHtml(dest)}</strong> due to road blockages.</p>
                        </div>
                    `;
                    graphVisualizer.resetHighlights();
                } else {
                    body.innerHTML = `
                        <div class="result-eta-badge">${res.totalDistance.toFixed(1)} minutes</div>
                        <p style="font-size: 0.85rem; color: var(--color-muted-slate); margin-bottom: 8px;">
                            ${res.path.length} locations traversed:
                        </p>
                        <div class="route-path-nodes">
                            ${res.path.map((p, idx) => `
                                <span class="route-node-pill">${escapeHtml(p)}</span>
                                ${idx < res.path.length - 1 ? '<span class="route-arrow">→</span>' : ''}
                            `).join('')}
                        </div>
                        <div style="font-size: 0.8rem; color: var(--color-muted-slate); margin-top: 10px;">
                            <strong>Segments:</strong> ${res.segments ? res.segments.join(' • ') : 'Direct transit'}
                        </div>
                    `;
                    graphVisualizer.highlightDijkstraPath(res.path);
                }
            }
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    document.getElementById('btn-run-bfs')?.addEventListener('click', async () => {
        const start = document.getElementById('route-start-select')?.value;
        if (!start) {
            showToast('Please select starting Origin location.', 'warning');
            return;
        }

        try {
            const data = await API.runBFS(start);
            const res = data.result;

            const card = document.getElementById('route-result-card');
            const title = document.getElementById('route-result-title');
            const body = document.getElementById('route-result-body');

            if (card && body) {
                card.classList.add('active');
                title.innerHTML = `BFS Reachable Zones from <strong>${escapeHtml(start)}</strong>`;
                body.innerHTML = `
                    <p style="font-size: 0.88rem; color: var(--color-charcoal); margin-bottom: 8px;">
                        Level-by-level network reachability exploration:
                    </p>
                    <div class="route-path-nodes">
                        ${res.traversalOrder.map((loc, idx) => `
                            <span class="route-node-pill">${escapeHtml(loc)}</span>
                            ${idx < res.traversalOrder.length - 1 ? '<span class="route-arrow">→</span>' : ''}
                        `).join('')}
                    </div>
                    <div style="font-size: 0.8rem; color: var(--color-muted-slate); margin-top: 10px;">
                        Reachable: ${res.visitedCount} of ${res.totalVertices} registered locations. Unreachable: ${res.unreachableCount}.
                    </div>
                `;
            }

            graphVisualizer.highlightTraversal(res.traversalOrder, start, res.unreachableLocations);
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    document.getElementById('btn-run-dfs')?.addEventListener('click', async () => {
        const start = document.getElementById('route-start-select')?.value;
        if (!start) {
            showToast('Please select starting Origin location.', 'warning');
            return;
        }

        try {
            const data = await API.runDFS(start);
            const res = data.result;

            const card = document.getElementById('route-result-card');
            const title = document.getElementById('route-result-title');
            const body = document.getElementById('route-result-body');

            if (card && body) {
                card.classList.add('active');
                title.innerHTML = `DFS Connectivity Traversal from <strong>${escapeHtml(start)}</strong>`;
                body.innerHTML = `
                    <p style="font-size: 0.88rem; color: var(--color-charcoal); margin-bottom: 8px;">
                        Branch connectivity exploration order:
                    </p>
                    <div class="route-path-nodes">
                        ${res.traversalOrder.map((loc, idx) => `
                            <span class="route-node-pill">${escapeHtml(loc)}</span>
                            ${idx < res.traversalOrder.length - 1 ? '<span class="route-arrow">→</span>' : ''}
                        `).join('')}
                    </div>
                    <div style="font-size: 0.8rem; color: var(--color-muted-slate); margin-top: 10px;">
                        Connected: ${res.visitedCount} of ${res.totalVertices} registered locations. Unreachable: ${res.unreachableCount}.
                    </div>
                `;
            }

            graphVisualizer.highlightTraversal(res.traversalOrder, start, res.unreachableLocations);
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    document.getElementById('btn-clear-route')?.addEventListener('click', () => {
        graphVisualizer.resetHighlights();
        const card = document.getElementById('route-result-card');
        if (card) card.classList.remove('active');
        showToast('Map highlights cleared.', 'info');
    });

    // -------------------------------------------------------------
    // 16. Render Section 9: Rescue Operations
    // -------------------------------------------------------------
    function renderOperationsTable() {
        const tbody = document.getElementById('operations-table-tbody');
        const badge = document.getElementById('badge-active-ops-count');
        if (!tbody) return;

        const activeReports = state.reports.filter(r => r.status === 'Assigned' || r.status === 'In Progress');
        if (badge) badge.textContent = `${activeReports.length} Active`;

        if (activeReports.length === 0) {
            tbody.innerHTML = '<tr><td colspan="7" class="empty-cell">No active missions in progress. All clear!</td></tr>';
            return;
        }

        tbody.innerHTML = activeReports.map(r => {
            const team = state.teams.find(t => t.id === r.assignedTeamId);
            return `
                <tr>
                    <td><strong>#${escapeHtml(r.id)}</strong></td>
                    <td><strong>${escapeHtml(r.location)}</strong></td>
                    <td>${escapeHtml(r.emergencyType)} (${escapeHtml(r.priority)})</td>
                    <td><strong>${escapeHtml(team ? team.name : r.assignedTeamId)}</strong> (${escapeHtml(r.assignedTeamId)})</td>
                    <td>${escapeHtml(team ? team.currentLocation : 'In Transit')}</td>
                    <td><span class="badge badge-${getStatusBadgeClass(r.status)}">${escapeHtml(r.status)}</span></td>
                    <td>
                        <div style="display: flex; gap: 6px; flex-wrap: wrap;">
                            ${r.status === 'Assigned' ? `
                                <button class="btn btn-primary btn-sm" onclick="window.appUpdateStatus('${r.id}', 'In Progress')">
                                    Start Mission
                                </button>
                            ` : ''}
                            ${r.status === 'In Progress' ? `
                                <button class="btn btn-success btn-sm" onclick="window.appUpdateStatus('${r.id}', 'Completed')">
                                    Complete & Release Team
                                </button>
                            ` : ''}
                            <button class="btn btn-danger btn-sm" onclick="window.appCancelMission('${r.id}')">
                                Cancel
                            </button>
                        </div>
                    </td>
                </tr>
            `;
        }).join('');
    }

    // -------------------------------------------------------------
    // 17. Render Section 10: Operation History
    // -------------------------------------------------------------
    function renderHistoryTable() {
        const tbody = document.getElementById('history-table-tbody');
        const badge = document.getElementById('badge-history-count');
        if (!tbody) return;

        if (badge) badge.textContent = `${state.history.length} Events`;

        if (state.history.length === 0) {
            tbody.innerHTML = '<tr><td colspan="4" class="empty-cell">No operational events recorded yet.</td></tr>';
            return;
        }

        tbody.innerHTML = state.history.map(op => `
            <tr>
                <td><small style="font-family: var(--font-mono);">${escapeHtml(op.timestamp)}</small></td>
                <td><span class="badge badge-${getCategoryBadge(op.category)}">${escapeHtml(op.category)}</span></td>
                <td>${escapeHtml(op.description)}</td>
                <td><small style="color: var(--color-muted-slate);">${escapeHtml(op.relatedId || '—')}</small></td>
            </tr>
        `).join('');
    }

    function getCategoryBadge(cat) {
        switch (cat) {
            case 'Request': return 'critical';
            case 'Resource': return 'moderate';
            case 'Allocation': return 'high';
            case 'Road': return 'warning';
            default: return 'low';
        }
    }

    document.getElementById('btn-history-load-all')?.addEventListener('click', async () => {
        const res = await API.getHistory('all');
        state.history = res.operations || [];
        renderHistoryTable();
        showToast('Full operation history loaded.', 'info');
    });

    // -------------------------------------------------------------
    // 18. Section 11: Settings & Persistence Handlers
    // -------------------------------------------------------------
    const saveHandlers = [
        document.getElementById('btn-header-save'),
        document.getElementById('btn-settings-save')
    ];

    saveHandlers.forEach(btn => {
        btn?.addEventListener('click', async () => {
            try {
                await API.saveData();
                showToast('All records saved successfully to data/ directory.', 'success');
            } catch (err) {
                showToast(err.message, 'error');
            }
        });
    });

    const resetDemoHandlers = [
        document.getElementById('btn-header-demo'),
        document.getElementById('btn-settings-reset-demo')
    ];

    resetDemoHandlers.forEach(btn => {
        btn?.addEventListener('click', async () => {
            if (!confirm('Reset to verified demonstration dataset? Active records will be restored to the standard demonstration state.')) return;
            try {
                await API.resetDemoData();
                showToast('Demonstration dataset restored successfully!', 'success');
                await loadAllData();
            } catch (err) {
                showToast(err.message, 'error');
            }
        });
    });

    document.getElementById('btn-settings-export')?.addEventListener('click', async () => {
        try {
            const data = await API.exportData();
            const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `resq_system_backup_${Date.now()}.json`;
            a.click();
            URL.revokeObjectURL(url);
            showToast('Complete system backup downloaded.', 'success');
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    document.getElementById('btn-header-refresh')?.addEventListener('click', async () => {
        showToast('Refreshing live data from C++ backend...', 'info');
        await loadAllData();
    });

    // -------------------------------------------------------------
    // 19. Form Submissions
    // -------------------------------------------------------------
    // Register Report Form
    document.getElementById('form-register-report')?.addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            personName: document.getElementById('report-name').value.trim(),
            location: document.getElementById('report-location').value,
            emergencyType: document.getElementById('report-emergency-type').value,
            priority: document.getElementById('report-priority').value,
            affectedPeople: parseInt(document.getElementById('report-affected').value) || 1,
            immediateRescueCount: parseInt(document.getElementById('report-immediate').value) || 0,
            requiredResourceType: document.getElementById('report-resource-type').value,
            requiredResourceQty: parseInt(document.getElementById('report-resource-qty').value) || 1,
            requiredCapability: document.getElementById('report-capability').value,
            description: document.getElementById('report-description').value.trim()
        };

        try {
            const res = await API.createReport(payload);
            showToast(`Report #${res.report.id} registered for ${payload.location}!`, 'success');
            closeModal('modal-register-report');
            e.target.reset();
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // Add Team Form
    document.getElementById('form-add-team')?.addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            name: document.getElementById('team-name').value.trim(),
            type: document.getElementById('team-type').value,
            currentLocation: document.getElementById('team-station').value,
            capacity: parseInt(document.getElementById('team-capacity').value) || 6,
            equipment: document.getElementById('team-equipment').value.trim() || 'Standard Equipment'
        };

        try {
            const res = await API.createTeam(payload);
            showToast(`Rescue Team ${res.team.id} (${res.team.name}) registered in fleet!`, 'success');
            closeModal('modal-add-team');
            e.target.reset();
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // Add / Replenish Resource Form
    document.getElementById('form-add-resource')?.addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            name: document.getElementById('res-name').value.trim(),
            category: document.getElementById('res-category').value.trim() || 'General Supplies',
            quantity: parseInt(document.getElementById('res-quantity').value) || 10,
            minThreshold: parseInt(document.getElementById('res-threshold').value) || 20,
            unit: document.getElementById('res-unit').value.trim() || 'units'
        };

        try {
            const res = await API.createResource(payload);
            showToast(`Stock updated for ${res.resource.name} (+${payload.quantity})`, 'success');
            closeModal('modal-add-resource');
            e.target.reset();
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // Allocate Resource Form
    document.getElementById('form-allocate-resource')?.addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            requestId: document.getElementById('alloc-report-id').value,
            resourceName: document.getElementById('alloc-resource-name').value,
            quantity: parseInt(document.getElementById('alloc-quantity').value) || 1,
            notes: document.getElementById('alloc-notes').value.trim()
        };

        try {
            const res = await API.allocateResource(payload);
            showToast(res.message, 'success');
            closeModal('modal-allocate-resource');
            e.target.reset();
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // Add Location Form
    document.getElementById('form-add-location')?.addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            name: document.getElementById('loc-name').value.trim(),
            type: document.getElementById('loc-type').value,
            x: parseFloat(document.getElementById('loc-x').value) || 400,
            y: parseFloat(document.getElementById('loc-y').value) || 250
        };

        try {
            const res = await API.createLocation(payload);
            showToast(`Location '${res.location.name}' registered in network!`, 'success');
            closeModal('modal-add-location');
            e.target.reset();
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // Add Road Form
    document.getElementById('form-add-road')?.addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            from: document.getElementById('road-from').value,
            to: document.getElementById('road-to').value,
            distance: parseFloat(document.getElementById('road-distance').value) || 5.0,
            status: document.getElementById('road-status').value
        };

        try {
            const res = await API.createRoad(payload);
            showToast(`Road '${res.road.from} ↔ ${res.road.to}' added!`, 'success');
            closeModal('modal-add-road');
            e.target.reset();
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    });

    // -------------------------------------------------------------
    // 20. Window Global Action Handlers
    // -------------------------------------------------------------
    window.appRecommendAssignment = async (reportId) => {
        try {
            const rec = await API.getAssignmentRecommendation(reportId);
            state.selectedRecommendation = rec;

            const modalBody = document.getElementById('recommendation-modal-body');
            if (!modalBody) return;

            const report = rec.report;
            const team = rec.recommendedTeam;
            const route = rec.route;
            const resCheck = rec.resourceCheck;

            modalBody.innerHTML = `
                <div style="background: var(--color-warm-cream); padding: 16px; border-radius: var(--radius-md); margin-bottom: 16px; border: 1px solid var(--color-border);">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
                        <h4 style="margin: 0; color: var(--color-charcoal); font-size: 1.1rem; font-weight: 800;">Emergency Report #${report.id}</h4>
                        <span class="badge badge-${getSeverityBadgeClass(report.priority)}">${escapeHtml(report.priority)}</span>
                    </div>
                    <div style="font-size: 0.9rem; line-height: 1.6; color: var(--color-charcoal);">
                        <div><strong>Location:</strong> ${escapeHtml(report.location)} • <strong>Disaster:</strong> ${escapeHtml(report.emergencyType)}</div>
                        <div><strong>Casualties:</strong> ${report.affectedPeople} affected (${report.immediateRescueCount} immediate rescue needed)</div>
                        <div><strong>Required Capability:</strong> ${escapeHtml(report.requiredCapability)}</div>
                        <div><strong>Required Supplies:</strong> ${report.requiredResourceQty} × ${escapeHtml(report.requiredResourceType)}</div>
                    </div>
                </div>

                ${team ? `
                    <div style="border: 1px solid var(--color-deep-blue); background: var(--color-deep-blue-light); padding: 16px; border-radius: var(--radius-md); margin-bottom: 16px;">
                        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 6px;">
                            <strong style="color: var(--color-deep-blue); font-size: 1rem; display: flex; align-items: center; gap: 6px;"><svg class="resq-icon"><use href="#icon-target"></use></svg> Recommended Rescue Team</strong>
                            <span class="badge badge-available">Available</span>
                        </div>
                        <div style="font-size: 0.9rem; line-height: 1.5; color: var(--color-charcoal);">
                            <div><strong>${escapeHtml(team.name)}</strong> (${escapeHtml(team.id)}) • ${escapeHtml(team.type)}</div>
                            <div><strong>Stationed At:</strong> ${escapeHtml(team.currentLocation)}</div>
                        </div>
                    </div>
                ` : `
                    <div style="background: var(--color-critical-red-light); border: 1px solid var(--color-critical-red); padding: 14px; border-radius: var(--radius-md); margin-bottom: 16px; color: var(--color-critical-red); font-size: 0.9rem;">
                        <div style="display: flex; align-items: flex-start; gap: 8px;"><svg class="resq-icon" style="margin-top: 2px;"><use href="#icon-alert-triangle"></use></svg> <div><strong>No Available Teams:</strong> All eligible teams with capability "${escapeHtml(report.requiredCapability)}" are currently busy on active missions.</div></div>
                    </div>
                `}

                ${route ? `
                    <div style="background: var(--color-warm-cream); padding: 16px; border-radius: var(--radius-md); margin-bottom: 16px; border: 1px solid var(--color-border);">
                        <strong style="color: var(--color-deep-blue); font-size: 0.95rem; display: flex; align-items: center; gap: 6px; margin-bottom: 6px;"><svg class="resq-icon"><use href="#icon-compass"></use></svg> Dijkstra Shortest Route Calculation</strong>
                        ${route.isReachable ? `
                            <div style="font-size: 0.9rem; line-height: 1.6;">
                                <div><strong>Estimated Travel Time:</strong> <span style="color: var(--color-success-green); font-weight: 700;">${route.estimatedTimeMinutes.toFixed(1)} minutes</span></div>
                                <div style="font-family: var(--font-mono); margin-top: 6px; font-weight: 600;">${route.path.join(' → ')}</div>
                            </div>
                        ` : `
                            <div style="color: var(--color-critical-red); font-size: 0.9rem; display: flex; align-items: center; gap: 6px;">
                                <svg class="resq-icon"><use href="#icon-ban"></use></svg> Destination is unreachable due to road blockages!
                            </div>
                        `}
                    </div>
                ` : ''}

                ${resCheck ? `
                    <div style="background: var(--color-warm-cream); padding: 16px; border-radius: var(--radius-md); margin-bottom: 16px; border: 1px solid var(--color-border);">
                        <strong style="color: var(--color-deep-blue); font-size: 0.95rem; display: flex; align-items: center; gap: 6px; margin-bottom: 6px;"><svg class="resq-icon"><use href="#icon-package"></use></svg> Resource Verification</strong>
                        ${resCheck.available ? `
                            <div style="color: var(--color-success-green); font-size: 0.9rem; font-weight: 600; display: flex; align-items: center; gap: 6px;">
                                <svg class="resq-icon"><use href="#icon-check-circle"></use></svg> Required supplies available (${resCheck.requestedStock} / ${resCheck.currentStock} ${resCheck.unit} in stock).
                            </div>
                        ` : `
                            <div style="color: var(--color-critical-red); font-size: 0.9rem; display: flex; align-items: flex-start; gap: 6px;">
                                <svg class="resq-icon" style="margin-top: 2px;"><use href="#icon-alert-triangle"></use></svg> <div><strong>Resource Shortage:</strong> Requested ${resCheck.requestedStock} ${resCheck.unit}, but only ${resCheck.currentStock} available in stock.</div>
                            </div>
                        `}
                    </div>
                ` : ''}

                <div style="display: flex; justify-content: flex-end; gap: 12px; margin-top: 20px;">
                    <button type="button" class="btn btn-secondary" data-close-modal="modal-recommendation">Cancel</button>
                    ${team && route && route.isReachable ? `
                        <button type="button" class="btn btn-primary" onclick="window.appConfirmAssignment('${report.id}', '${team.id}')">
                            Confirm Mission Assignment
                        </button>
                    ` : `
                        <button type="button" class="btn btn-secondary" disabled>
                            Assignment Unavailable
                        </button>
                    `}
                </div>
            `;

            modalBody.querySelectorAll('[data-close-modal]').forEach(b => {
                b.addEventListener('click', () => closeModal('modal-recommendation'));
            });

            openModal('modal-recommendation');
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    window.appConfirmAssignment = async (reportId, teamId) => {
        try {
            const res = await API.confirmAssignment(reportId, teamId);
            showToast(`Mission confirmed! Team dispatched with ETA: ${res.etaMinutes.toFixed(0)} min.`, 'success');
            closeModal('modal-recommendation');
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    window.appUpdateStatus = async (reportId, newStatus) => {
        try {
            await API.updateAssignmentStatus(reportId, newStatus);
            showToast(`Report #${reportId} status updated to '${newStatus}'.`, 'success');
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    window.appCancelMission = async (reportId) => {
        if (!confirm(`Are you sure you want to cancel operation for Report #${reportId}?`)) return;
        try {
            await API.updateAssignmentStatus(reportId, 'Cancelled');
            showToast(`Operation #${reportId} has been cancelled.`, 'info');
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    window.appEscalateReport = async (reportId) => {
        try {
            await API.escalateReport(reportId);
            showToast(`Report #${reportId} escalated to CRITICAL urgency!`, 'warning');
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    window.appReleaseTeam = async (teamId) => {
        try {
            await API.releaseTeam(teamId);
            showToast(`Team ${teamId} returned to Available status.`, 'success');
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    window.appToggleRoadBlock = async (roadId) => {
        try {
            const res = await API.toggleRoadBlock(roadId);
            const road = res.road;
            showToast(`Road ${road.from} ↔ ${road.to} is now ${road.status.toUpperCase()}!`, road.status === 'Blocked' ? 'warning' : 'success');
            await loadAllData();
        } catch (err) {
            showToast(err.message, 'error');
        }
    };

    window.appQuickReplenish = (resourceName) => {
        document.getElementById('res-name').value = resourceName;
        document.getElementById('res-quantity').value = 50;
        openModal('modal-add-resource');
    };

    // -------------------------------------------------------------
    // 21. Initial Boot & Auto-Poll Loop
    // -------------------------------------------------------------
    loadAllData();
    setInterval(loadAllData, 10000); // Poll every 10 seconds
});
