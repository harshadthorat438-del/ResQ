/**
 * SVG Graph Visualization Module for ResQ — Disaster Rescue Network
 * Renders nodes, edges, travel times, blocked-road markers, and operational route paths
 * Optimized for both Humanitarian Cream/White and high-contrast dashboards
 */

class DisasterGraphVisualizer {
    constructor(svgElementId) {
        this.svg = typeof svgElementId === 'string' ? document.getElementById(svgElementId) : svgElementId;
        this.locations = [];
        this.roads = [];
        this.nodePositions = new Map();
        this.animationTimer = null;
        this.onNodeClickCallback = null;
    }

    onNodeClick(callback) {
        this.onNodeClickCallback = callback;
    }

    setData(locations, roads) {
        this.locations = locations || [];
        this.roads = roads || [];
        this.computeLayout();
        this.render();
    }

    computeLayout() {
        this.nodePositions.clear();
        if (!this.svg) return;
        const width = this.svg.clientWidth || 800;
        const height = this.svg.clientHeight || 450;
        const padding = 65;

        const count = this.locations.length;
        if (count === 0) return;

        let hasCustomCoords = this.locations.some(l => l.x > 0 || l.y > 0);

        if (hasCustomCoords) {
            let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
            this.locations.forEach(l => {
                if (l.x < minX) minX = l.x;
                if (l.x > maxX) maxX = l.x;
                if (l.y < minY) minY = l.y;
                if (l.y > maxY) maxY = l.y;
            });

            const spanX = (maxX - minX) || 1;
            const spanY = (maxY - minY) || 1;

            this.locations.forEach(l => {
                const nx = padding + ((l.x - minX) / spanX) * (width - 2 * padding);
                const ny = padding + ((l.y - minY) / spanY) * (height - 2 * padding);
                this.nodePositions.set(l.name, { x: nx, y: ny, data: l });
            });
        } else {
            const centerX = width / 2;
            const centerY = height / 2;
            const radiusX = (width / 2) - padding;
            const radiusY = (height / 2) - padding;

            this.locations.forEach((loc, idx) => {
                const angle = (2 * Math.PI * idx) / count;
                const x = centerX + radiusX * Math.cos(angle);
                const y = centerY + radiusY * Math.sin(angle);
                this.nodePositions.set(loc.name, { x, y, data: loc });
            });
        }
    }

    render() {
        if (!this.svg) return;
        this.svg.innerHTML = '';

        if (this.locations.length === 0) {
            this.svg.innerHTML = `
                <text x="50%" y="50%" text-anchor="middle" fill="#64748B" font-size="14" font-family="Plus Jakarta Sans">
                    No locations registered in the disaster response network.
                </text>
            `;
            return;
        }

        // Defs for filters and glow effects
        const defs = document.createElementNS('http://www.w3.org/2000/svg', 'defs');
        defs.innerHTML = `
            <filter id="glow-teal" x="-20%" y="-20%" width="140%" height="140%">
                <feGaussianBlur stdDeviation="3" result="blur" />
                <feComposite in="SourceGraphic" in2="blur" operator="over" />
            </filter>
            <filter id="glow-blue" x="-20%" y="-20%" width="140%" height="140%">
                <feGaussianBlur stdDeviation="4" result="blur" />
                <feComposite in="SourceGraphic" in2="blur" operator="over" />
            </filter>
            <filter id="glow-red" x="-20%" y="-20%" width="140%" height="140%">
                <feGaussianBlur stdDeviation="4" result="blur" />
                <feComposite in="SourceGraphic" in2="blur" operator="over" />
            </filter>
        `;
        this.svg.appendChild(defs);

        // Group for Edges / Roads
        const edgesGroup = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        edgesGroup.setAttribute('id', `graph-edges-${this.svg.id || 'main'}`);
        this.svg.appendChild(edgesGroup);

        // Render Roads (Edges)
        this.roads.forEach(road => {
            const posU = this.nodePositions.get(road.from);
            const posV = this.nodePositions.get(road.to);
            if (!posU || !posV) return;

            const edgeId = `edge-${this.cleanId(road.from)}-${this.cleanId(road.to)}`;
            const isBlocked = road.status === 'Blocked';
            const isHazardous = road.status === 'Hazardous';

            const strokeColor = isBlocked ? '#C83D3D' : (isHazardous ? '#E3C71F' : '#64748B');
            const strokeWidth = isBlocked ? '3.5' : '2.5';

            const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            line.setAttribute('x1', posU.x);
            line.setAttribute('y1', posU.y);
            line.setAttribute('x2', posV.x);
            line.setAttribute('y2', posV.y);
            line.setAttribute('stroke', strokeColor);
            line.setAttribute('stroke-width', strokeWidth);
            line.setAttribute('stroke-linecap', 'round');
            if (isBlocked) {
                line.setAttribute('stroke-dasharray', '8,4');
            }
            line.setAttribute('id', edgeId);
            edgesGroup.appendChild(line);

            // Midpoint label for road travel time
            const midX = (posU.x + posV.x) / 2;
            const midY = (posU.y + posV.y) / 2;

            // Background pill for label
            const textBg = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            const labelText = isBlocked ? `BLOCKED (${road.distance} min)` : `${road.distance} min`;
            const textWidth = labelText.length * 7 + 14;
            textBg.setAttribute('x', midX - textWidth / 2);
            textBg.setAttribute('y', midY - 12);
            textBg.setAttribute('width', textWidth);
            textBg.setAttribute('height', '20');
            textBg.setAttribute('rx', '6');
            textBg.setAttribute('fill', isBlocked ? '#FBEBEB' : '#FFFFFF');
            textBg.setAttribute('stroke', isBlocked ? '#C83D3D' : 'rgba(52, 51, 49, 0.15)');
            textBg.setAttribute('stroke-width', '1');
            edgesGroup.appendChild(textBg);

            const distLabel = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            distLabel.setAttribute('x', midX);
            distLabel.setAttribute('y', midY + 2);
            distLabel.setAttribute('text-anchor', 'middle');
            distLabel.setAttribute('fill', isBlocked ? '#C83D3D' : '#343331');
            distLabel.setAttribute('font-size', '10.5');
            distLabel.setAttribute('font-weight', isBlocked ? '700' : '600');
            distLabel.setAttribute('font-family', 'Plus Jakarta Sans');
            distLabel.textContent = labelText;
            edgesGroup.appendChild(distLabel);
        });

        // Group for Nodes / Locations
        const nodesGroup = document.createElementNS('http://www.w3.org/2000/svg', 'g');
        nodesGroup.setAttribute('id', `graph-nodes-${this.svg.id || 'main'}`);
        this.svg.appendChild(nodesGroup);

        // Render Locations (Vertices)
        this.nodePositions.forEach((pos, name) => {
            const nodeId = `node-${this.cleanId(name)}`;
            const group = document.createElementNS('http://www.w3.org/2000/svg', 'g');
            group.setAttribute('id', nodeId);
            group.setAttribute('transform', `translate(${pos.x}, ${pos.y})`);
            group.style.cursor = 'pointer';

            group.addEventListener('click', () => {
                if (this.onNodeClickCallback) {
                    this.onNodeClickCallback(name, pos.data);
                }
            });

            // Outer pulse circle
            const pulse = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            pulse.setAttribute('r', '28');
            pulse.setAttribute('fill', 'none');
            pulse.setAttribute('stroke', '#245B78');
            pulse.setAttribute('stroke-width', '2');
            pulse.setAttribute('opacity', '0');
            pulse.setAttribute('class', 'node-pulse');
            group.appendChild(pulse);

            // Node Circle Base
            const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            circle.setAttribute('r', '22');
            circle.setAttribute('fill', this.getNodeColor(pos.data.type));
            circle.setAttribute('stroke', '#FFFFFF');
            circle.setAttribute('stroke-width', '3');
            circle.setAttribute('class', 'node-circle');
            group.appendChild(circle);

            // Node Icon (Vector SVG via symbol use)
            const iconUse = document.createElementNS('http://www.w3.org/2000/svg', 'use');
            iconUse.setAttribute('href', this.getNodeIconId(pos.data.type));
            iconUse.setAttribute('x', '-9');
            iconUse.setAttribute('y', '-9');
            iconUse.setAttribute('width', '18');
            iconUse.setAttribute('height', '18');
            iconUse.setAttribute('stroke', '#FFFFFF');
            iconUse.setAttribute('stroke-width', '2.2');
            iconUse.setAttribute('fill', 'none');
            group.appendChild(iconUse);

            // Node Label
            const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            label.setAttribute('x', '0');
            label.setAttribute('y', '34');
            label.setAttribute('text-anchor', 'middle');
            label.setAttribute('fill', '#343331');
            label.setAttribute('font-size', '12');
            label.setAttribute('font-weight', '700');
            label.setAttribute('font-family', 'Plus Jakarta Sans');
            label.textContent = name;
            group.appendChild(label);

            // Type sub-label
            const typeLabel = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            typeLabel.setAttribute('x', '0');
            typeLabel.setAttribute('y', '47');
            typeLabel.setAttribute('text-anchor', 'middle');
            typeLabel.setAttribute('fill', '#64748B');
            typeLabel.setAttribute('font-size', '9.5');
            typeLabel.setAttribute('font-weight', '600');
            typeLabel.setAttribute('font-family', 'Plus Jakarta Sans');
            typeLabel.textContent = pos.data.type;
            group.appendChild(typeLabel);

            nodesGroup.appendChild(group);
        });
    }

    highlightDijkstraPath(path) {
        this.resetHighlights();
        if (!path || path.length < 2) return;

        path.forEach((nodeName, idx) => {
            const nodeEl = this.svg.querySelector(`#node-${this.cleanId(nodeName)}`);
            if (nodeEl) {
                const circle = nodeEl.querySelector('.node-circle');
                if (circle) {
                    if (idx === 0) {
                        circle.setAttribute('stroke', '#245B78'); // Origin
                        circle.setAttribute('stroke-width', '5');
                    } else if (idx === path.length - 1) {
                        circle.setAttribute('stroke', '#C83D3D'); // Destination
                        circle.setAttribute('stroke-width', '5');
                    } else {
                        circle.setAttribute('stroke', '#287A58'); // Waypoint
                        circle.setAttribute('stroke-width', '4');
                    }
                }
            }

            if (idx > 0) {
                const prev = path[idx - 1];
                const id1 = `edge-${this.cleanId(prev)}-${this.cleanId(nodeName)}`;
                const id2 = `edge-${this.cleanId(nodeName)}-${this.cleanId(prev)}`;
                const line = this.svg.querySelector(`#${id1}`) || this.svg.querySelector(`#${id2}`);
                if (line) {
                    line.setAttribute('stroke', '#287A58');
                    line.setAttribute('stroke-width', '5');
                    line.setAttribute('stroke-dasharray', '8 4');
                }
            }
        });
    }

    highlightTraversal(traversalOrder, startLocation, unreachableLocations) {
        this.resetHighlights();
        if (this.animationTimer) clearInterval(this.animationTimer);

        const startNode = this.svg.querySelector(`#node-${this.cleanId(startLocation)}`);
        if (startNode) {
            const circle = startNode.querySelector('.node-circle');
            if (circle) {
                circle.setAttribute('stroke', '#245B78');
                circle.setAttribute('stroke-width', '5');
            }
        }

        let step = 0;
        this.animationTimer = setInterval(() => {
            if (step >= traversalOrder.length) {
                clearInterval(this.animationTimer);
                if (unreachableLocations && unreachableLocations.length > 0) {
                    unreachableLocations.forEach(unreachName => {
                        const unNode = this.svg.querySelector(`#node-${this.cleanId(unreachName)}`);
                        if (unNode) {
                            const c = unNode.querySelector('.node-circle');
                            if (c) {
                                c.setAttribute('stroke', '#C83D3D');
                                c.setAttribute('stroke-width', '4');
                                c.setAttribute('stroke-dasharray', '4 2');
                            }
                        }
                    });
                }
                return;
            }

            const currentName = traversalOrder[step];
            const nodeEl = this.svg.querySelector(`#node-${this.cleanId(currentName)}`);
            if (nodeEl && currentName !== startLocation) {
                const circle = nodeEl.querySelector('.node-circle');
                if (circle) {
                    circle.setAttribute('stroke', '#287A58');
                    circle.setAttribute('stroke-width', '4');
                }
            }

            if (step > 0) {
                const prevName = traversalOrder[step - 1];
                this.highlightEdge(prevName, currentName);
            }

            step++;
        }, 320);
    }

    highlightEdge(from, to) {
        const id1 = `edge-${this.cleanId(from)}-${this.cleanId(to)}`;
        const id2 = `edge-${this.cleanId(to)}-${this.cleanId(from)}`;
        const line = this.svg.querySelector(`#${id1}`) || this.svg.querySelector(`#${id2}`);
        if (line) {
            line.setAttribute('stroke', '#287A58');
            line.setAttribute('stroke-width', '4.5');
        }
    }

    resetHighlights() {
        if (this.animationTimer) clearInterval(this.animationTimer);
        this.render();
    }

    getNodeColor(type) {
        switch (type) {
            case 'Rescue Station': return '#245B78'; // Deep Blue
            case 'Relief Camp': return '#287A58';    // Success Green
            case 'Hospital':
            case 'Medical Center': return '#C83D3D'; // Critical Red
            case 'Affected Area':
            case 'Village': return '#E3C71F';        // Rescue Yellow
            case 'Bridge Checkpoint': return '#4A4845'; // Charcoal
            default: return '#245B78';
        }
    }

    getNodeIconId(type) {
        switch (type) {
            case 'Rescue Station': return '#icon-shield';
            case 'Relief Camp': return '#icon-users';
            case 'Hospital':
            case 'Medical Center': return '#icon-activity';
            case 'Affected Area':
            case 'Village': return '#icon-map-pin';
            case 'Bridge Checkpoint': return '#icon-route';
            default: return '#icon-map-pin';
        }
    }

    cleanId(str) {
        return (str || '').replace(/[^a-zA-Z0-9]/g, '_');
    }
}

window.DisasterGraphVisualizer = DisasterGraphVisualizer;
