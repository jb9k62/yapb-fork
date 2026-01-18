// Solution for Lesson 13: Navigation & A* Pathfinding
//
// Debug output for pathfinding operations.

// ============================================================
// MODIFIED findPath in src/navigate.cpp
// ============================================================

void Bot::findPath(int srcIndex, int destIndex, FindPath pathType) {
    // Validate inputs
    if (!graph.exists(srcIndex) || !graph.exists(destIndex)) {
        ctrl.msg("%s: Invalid path request (src=%d, dest=%d)",
                 pev->netname.chars(), srcIndex, destIndex);
        return;
    }

    // Skip if already at destination
    if (srcIndex == destIndex) {
        m_pathWalk.clear();
        return;
    }

    // Debug: Log request
    const char* pathTypeName = "Unknown";
    switch (pathType) {
        case FindPath::Fast:    pathTypeName = "Fast";    break;
        case FindPath::Optimal: pathTypeName = "Optimal"; break;
        case FindPath::Safe:    pathTypeName = "Safe";    break;
    }

    ctrl.msg("%s: Path request %d→%d (%s)",
             pev->netname.chars(), srcIndex, destIndex, pathTypeName);

    // Measure time
    float startTime = game.time();

    // Perform pathfinding
    bool found = m_planner->find(srcIndex, destIndex);

    float elapsedMs = (game.time() - startTime) * 1000.0f;

    if (found) {
        m_planner->getPath(m_pathWalk);

        // Calculate total path distance
        float totalDist = 0.0f;
        for (size_t i = 0; i < m_pathWalk.length() - 1; i++) {
            Vector from = graph[m_pathWalk.at(i)].origin;
            Vector to = graph[m_pathWalk.at(i + 1)].origin;
            totalDist += (to - from).length();
        }

        ctrl.msg("%s: Path found! %d nodes, %.0f units, %.2f ms",
                 pev->netname.chars(),
                 static_cast<int>(m_pathWalk.length()),
                 totalDist,
                 elapsedMs);

        // Print first few nodes
        if (m_pathWalk.length() <= 5) {
            String path;
            for (size_t i = 0; i < m_pathWalk.length(); i++) {
                path.appendFormat("%d ", m_pathWalk.at(i));
            }
            ctrl.msg("  Path: %s", path.chars());
        }
    } else {
        ctrl.msg("%s: NO PATH FOUND from %d to %d! (%.2f ms)",
                 pev->netname.chars(), srcIndex, destIndex, elapsedMs);
    }
}

// ============================================================
// PATH VISUALIZATION
// ============================================================

void Bot::debugDrawPath() {
    // Only draw if we have editor rights
    if (!graph.hasEditor()) return;
    if (m_pathWalk.empty()) return;

    edict_t* editor = graph.getEditor();

    for (size_t i = 0; i < m_pathWalk.length() - 1; i++) {
        int nodeA = m_pathWalk.at(i);
        int nodeB = m_pathWalk.at(i + 1);

        Vector from = graph[nodeA].origin;
        Vector to = graph[nodeB].origin;

        // Green line for path
        game.drawLine(editor, from, to,
                      5, 0, Color(0, 255, 0), 255, 0, 10,
                      DrawLine::Simple);

        // Mark current node in yellow
        if (i == 0) {
            game.drawLine(editor, from, from + Vector(0, 0, 50),
                          10, 0, Color(255, 255, 0), 255, 0, 10);
        }
    }

    // Mark goal in red
    Vector goal = graph[m_pathWalk.last()].origin;
    game.drawLine(editor, goal, goal + Vector(0, 0, 50),
                  10, 0, Color(255, 0, 0), 255, 0, 10);
}

// ============================================================
// PATH STATISTICS
// ============================================================

void Bot::printPathStats() {
    if (m_pathWalk.empty()) {
        ctrl.msg("%s: No current path", pev->netname.chars());
        return;
    }

    ctrl.msg("%s Path Statistics:", pev->netname.chars());
    ctrl.msg("  Nodes remaining: %d", static_cast<int>(m_pathWalk.length()));
    ctrl.msg("  Current node: %d", m_currentNodeIndex);

    if (m_pathWalk.length() > 0) {
        ctrl.msg("  Next node: %d", m_pathWalk.first());
        ctrl.msg("  Final node: %d", m_pathWalk.last());
    }

    // Calculate remaining distance
    float remaining = 0.0f;
    Vector current = pev->origin;

    for (size_t i = 0; i < m_pathWalk.length(); i++) {
        Vector nodePos = graph[m_pathWalk.at(i)].origin;
        remaining += (nodePos - current).length();
        current = nodePos;
    }

    ctrl.msg("  Distance remaining: %.0f units", remaining);
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class Bot {
    findPath(srcIndex: number, destIndex: number, pathType: FindPath): void {
        if (!graph.exists(srcIndex) || !graph.exists(destIndex)) {
            console.log(`${this.name}: Invalid path request`);
            return;
        }

        console.log(`${this.name}: Path request ${srcIndex}→${destIndex}`);

        const startTime = performance.now();
        const found = this.planner.find(srcIndex, destIndex);
        const elapsed = performance.now() - startTime;

        if (found) {
            this.path = this.planner.getPath();
            console.log(
                `${this.name}: Path found! ${this.path.length} nodes, ` +
                `${elapsed.toFixed(2)} ms`
            );
        } else {
            console.log(`${this.name}: NO PATH FOUND!`);
        }
    }
}
*/

// ============================================================
// A* ALGORITHM EXPLAINED
// ============================================================

/*
A* maintains two scores for each node:
- g(n): Actual cost from start to node n
- f(n): Estimated total cost: g(n) + h(n)

h(n) is the heuristic - estimated cost from n to goal.
In YaPB, this is usually Euclidean distance.

The algorithm always expands the node with lowest f(n),
which guarantees finding the optimal path if h(n) never
overestimates the true cost (admissible heuristic).

Example run:
  Start=0, Goal=5
  Nodes: 0, 1, 2, 3, 4, 5

  Step 1: Expand 0
          Found 1 (g=10, h=50, f=60)
          Open: {1}

  Step 2: Expand 1 (lowest f)
          Found 2 (g=20, h=40, f=60)
          Found 4 (g=15, h=30, f=45)
          Open: {2, 4}

  Step 3: Expand 4 (lowest f=45)
          Found 5 (g=25, h=0, f=25) ← GOAL!

  Path: 0 → 1 → 4 → 5
*/
