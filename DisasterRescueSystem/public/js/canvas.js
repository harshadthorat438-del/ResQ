/**
 * Graph Canvas Renderer with Draggable Nodes & Traversal Animation
 */

class GraphCanvas {
    constructor(canvasId) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.nodes = []; // { name, x, y, radius, isVisited, isCurrent }
        this.roads = []; // [ [uName, vName], ... ]
        this.activeTraversedEdges = new Set(); // "u|v"

        this.draggedNode = null;
        this.offsetX = 0;
        this.offsetY = 0;

        this.initEvents();
        this.resize();
        window.addEventListener('resize', () => this.resize());
    }

    resize() {
        const rect = this.canvas.parentElement.getBoundingClientRect();
        this.canvas.width = rect.width || 700;
        this.canvas.height = 380;
        this.render();
    }

    setGraph(locations, roads) {
        this.roads = roads;
        const width = this.canvas.width;
        const height = this.canvas.height;
        const centerX = width / 2;
        const centerY = height / 2;
        const radius = Math.min(width, height) * 0.35;

        // Keep existing positions if node already existed, else distribute in circle
        const oldPos = new Map();
        for (const n of this.nodes) {
            oldPos.set(n.name, { x: n.x, y: n.y });
        }

        this.nodes = locations.map((loc, i) => {
            if (oldPos.has(loc)) {
                const p = oldPos.get(loc);
                return { name: loc, x: p.x, y: p.y, radius: 24, isVisited: false, isCurrent: false };
            }
            const angle = (2 * Math.PI * i) / (locations.length || 1);
            return {
                name: loc,
                x: centerX + radius * Math.cos(angle),
                y: centerY + radius * Math.sin(angle),
                radius: 24,
                isVisited: false,
                isCurrent: false
            };
        });

        this.render();
    }

    clearHighlights() {
        for (const n of this.nodes) {
            n.isVisited = false;
            n.isCurrent = false;
        }
        this.activeTraversedEdges.clear();
        this.render();
    }

    highlightStep(currentName, fromName, visitedList) {
        for (const n of this.nodes) {
            n.isVisited = visitedList.includes(n.name);
            n.isCurrent = (n.name === currentName);
        }

        if (fromName && currentName) {
            this.activeTraversedEdges.add(`${fromName}|${currentName}`);
            this.activeTraversedEdges.add(`${currentName}|${fromName}`);
        }

        this.render();
    }

    getNode(name) {
        return this.nodes.find(n => n.name === name);
    }

    render() {
        const ctx = this.ctx;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        // 1. Draw Grid Pattern
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.03)';
        ctx.lineWidth = 1;
        const gridSize = 30;
        for (let x = 0; x < this.canvas.width; x += gridSize) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, this.canvas.height);
            ctx.stroke();
        }
        for (let y = 0; y < this.canvas.height; y += gridSize) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(this.canvas.width, y);
            ctx.stroke();
        }

        // 2. Draw Roads (Edges)
        for (const road of this.roads) {
            const u = this.getNode(road[0]);
            const v = this.getNode(road[1]);
            if (!u || !v) continue;

            const isTraversed = this.activeTraversedEdges.has(`${u.name}|${v.name}`);

            ctx.beginPath();
            ctx.moveTo(u.x, u.y);
            ctx.lineTo(v.x, v.y);

            if (isTraversed) {
                ctx.strokeStyle = '#10b981';
                ctx.lineWidth = 4;
                ctx.shadowColor = '#10b981';
                ctx.shadowBlur = 10;
            } else {
                ctx.strokeStyle = '#2f4368';
                ctx.lineWidth = 2;
                ctx.shadowBlur = 0;
            }
            ctx.stroke();
            ctx.shadowBlur = 0;
        }

        // 3. Draw Nodes (Vertices)
        for (const node of this.nodes) {
            ctx.save();

            // Glow / Pulse effect
            if (node.isCurrent) {
                ctx.shadowColor = '#f59e0b';
                ctx.shadowBlur = 20;
            } else if (node.isVisited) {
                ctx.shadowColor = '#10b981';
                ctx.shadowBlur = 14;
            } else {
                ctx.shadowColor = 'rgba(0, 0, 0, 0.5)';
                ctx.shadowBlur = 6;
            }

            // Fill Circle
            ctx.beginPath();
            ctx.arc(node.x, node.y, node.radius, 0, Math.PI * 2);

            if (node.isCurrent) {
                ctx.fillStyle = '#f59e0b';
            } else if (node.isVisited) {
                ctx.fillStyle = '#10b981';
            } else {
                ctx.fillStyle = '#1e3a8a';
            }
            ctx.fill();

            // Border Circle
            ctx.lineWidth = 3;
            if (node.isCurrent) {
                ctx.strokeStyle = '#ffffff';
            } else if (node.isVisited) {
                ctx.strokeStyle = '#a7f3d0';
            } else {
                ctx.strokeStyle = '#38bdf8';
            }
            ctx.stroke();

            // Node Icon or Dot
            ctx.fillStyle = '#ffffff';
            ctx.font = 'bold 12px "JetBrains Mono", monospace';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(node.name.charAt(0), node.x, node.y);

            // Node Label
            ctx.shadowBlur = 0;
            ctx.font = '600 11px "Plus Jakarta Sans", sans-serif';
            ctx.fillStyle = node.isCurrent ? '#fbbf24' : (node.isVisited ? '#34d399' : '#e2e8f0');
            ctx.fillText(node.name, node.x, node.y + node.radius + 15);

            ctx.restore();
        }
    }

    initEvents() {
        const getPos = (e) => {
            const rect = this.canvas.getBoundingClientRect();
            const scaleX = this.canvas.width / rect.width;
            const scaleY = this.canvas.height / rect.height;
            return {
                x: (e.clientX - rect.left) * scaleX,
                y: (e.clientY - rect.top) * scaleY
            };
        };

        this.canvas.addEventListener('mousedown', (e) => {
            const pos = getPos(e);
            for (let i = this.nodes.length - 1; i >= 0; i--) {
                const n = this.nodes[i];
                const dx = pos.x - n.x;
                const dy = pos.y - n.y;
                if (Math.hypot(dx, dy) <= n.radius) {
                    this.draggedNode = n;
                    this.offsetX = dx;
                    this.offsetY = dy;
                    return;
                }
            }
        });

        window.addEventListener('mousemove', (e) => {
            if (!this.draggedNode) return;
            const pos = getPos(e);
            this.draggedNode.x = Math.max(30, Math.min(this.canvas.width - 30, pos.x - this.offsetX));
            this.draggedNode.y = Math.max(30, Math.min(this.canvas.height - 30, pos.y - this.offsetY));
            this.render();
        });

        window.addEventListener('mouseup', () => {
            this.draggedNode = null;
        });
    }
}
