# Lesson 13: Navigation & A* Pathfinding

Understand how bots navigate the game world using node graphs and the A* algorithm.

## Learning Objectives

- Understand navigation graphs (waypoints/nodes)
- Learn the A* pathfinding algorithm
- Trace a path request in YaPB
- Add pathfinding debug output

## Theory: Navigation Graph

Bots don't understand the 3D map geometry. Instead, they navigate using a **graph** of connected nodes (waypoints).

### The Graph Structure

```
     [2]───────[3]
      │╲        │
      │ ╲       │
      │  ╲      │
     [1]──[4]──[5]
      │
     [0]

Nodes: Positions in the map
Edges: Connections between nodes (walkable paths)
```

### TypeScript Equivalent

```typescript
interface Node {
    id: number;
    position: Vector;
    flags: number;  // Ladder, Camp spot, etc.
    links: Link[];
}

interface Link {
    targetNode: number;
    distance: number;
    flags: number;  // Jump required, etc.
}

const graph: Node[] = [];
```

### C++ Structure

```cpp
// File: inc/graph.h
struct Path {
    int32_t number {};
    int32_t flags {};    // NodeFlag::Ladder, ::Camp, etc.
    Vector origin {};    // Node position
    Vector start {}, end {}; // For camp nodes
    float radius {};
    PathLink links[kMaxNodeLinks] {};  // Up to 8 connections
};

struct PathLink {
    Vector velocity {};   // For jump connections
    int32_t distance {};  // Distance to target
    uint16_t flags {};    // PathFlag::Jump, etc.
    int16_t index {};     // Target node index
};
```

## Theory: A* Algorithm

A* finds the shortest path between two nodes.

### The Basic Idea

```
1. Start with start node in "open set"
2. Pick node with lowest f(n) = g(n) + h(n)
   - g(n) = actual cost from start
   - h(n) = estimated cost to goal (heuristic)
3. If it's the goal, we're done
4. Otherwise, explore its neighbors
5. Repeat until goal found or open set empty
```

### Visual Example

```
Start: Node 0
Goal: Node 5

Step 1: Open = [0], evaluate neighbors
        Found: 1 (g=10, h=50, f=60)
        Open = [1]

Step 2: Open = [1], evaluate neighbors
        Found: 2 (g=20, h=40, f=60)
        Found: 4 (g=15, h=30, f=45) ← Lowest f!
        Open = [2, 4]

Step 3: Open = [2, 4], pick 4 (lowest f)
        Found: 5 (g=25, h=0, f=25) ← GOAL!

Path: 0 → 1 → 4 → 5
```

### Heuristic Function

The heuristic estimates remaining distance. YaPB uses Euclidean distance:

```cpp
float heuristic(int node, int goal) {
    Vector nodePos = graph[node].origin;
    Vector goalPos = graph[goal].origin;
    return (goalPos - nodePos).length();
}
```

## Code Study: BotGraph Class

### File: `inc/graph.h:148-391`

```cpp
class BotGraph final : public Singleton<BotGraph> {
public:
    SmallArray<Path> m_paths {};  // All nodes
    HashMap<int32_t, Array<int32_t>> m_hashTable {}; // Spatial lookup

    // Get node by index
    Path& operator[](int index) {
        return m_paths[index];
    }

    // Number of nodes
    int32_t length() const {
        return m_paths.length<int32_t>();
    }

    // Find nearest node to position
    int getNearest(const Vector& origin, float range = kInfiniteDistance);
};
```

### File: `inc/planner.h:87-160` (A* Implementation)

```cpp
class AStarAlgo final {
    // Priority queue for open set
    // Stores nodes to explore, sorted by f value

public:
    bool find(int srcIndex, int destIndex);
    void getPath(PathWalk& path);
};
```

## Code Study: Path Request Flow

### Bot Requests Path

```cpp
// In Bot::findPath (src/navigate.cpp)
void Bot::findPath(int srcIndex, int destIndex, FindPath pathType) {
    // Use the A* planner
    m_planner->find(srcIndex, destIndex);

    // Get resulting path
    m_planner->getPath(m_pathWalk);
}
```

### A* Execution

```cpp
bool AStarAlgo::find(int src, int dest) {
    // Initialize
    clearData();
    addToOpenSet(src, 0, heuristic(src, dest));

    while (!openSet.empty()) {
        int current = getLowestF();

        if (current == dest) {
            reconstructPath();
            return true;
        }

        // Explore neighbors
        Path& node = graph[current];
        for (auto& link : node.links) {
            if (link.index < 0) continue;

            float newG = g[current] + link.distance;
            if (newG < g[link.index]) {
                cameFrom[link.index] = current;
                g[link.index] = newG;
                f[link.index] = newG + heuristic(link.index, dest);
                addToOpenSet(link.index);
            }
        }
    }
    return false;  // No path found
}
```

## Exercise: Add Pathfinding Debug

Add debug output that shows pathfinding requests and results.

### Your Task

1. Log when a path is requested
2. Log the path length and nodes
3. Print warnings for failed path requests

### Implementation

In `src/navigate.cpp`, modify `findPath`:

```cpp
void Bot::findPath(int srcIndex, int destIndex, FindPath pathType) {
    // Debug: Log request
    ctrl.msg("%s: Finding path from node %d to %d",
             pev->netname.chars(), srcIndex, destIndex);

    float startTime = game.time();

    // Actual pathfinding
    bool found = m_planner->find(srcIndex, destIndex);

    float elapsed = (game.time() - startTime) * 1000.0f;

    if (found) {
        m_planner->getPath(m_pathWalk);
        ctrl.msg("%s: Path found! %d nodes, %.2f ms",
                 pev->netname.chars(),
                 static_cast<int>(m_pathWalk.length()),
                 elapsed);
    } else {
        ctrl.msg("%s: NO PATH from %d to %d!",
                 pev->netname.chars(), srcIndex, destIndex);
    }
}
```

### Visualize the Path

Add optional path drawing:

```cpp
void Bot::debugDrawPath() {
    if (m_pathWalk.empty()) return;

    for (size_t i = 0; i < m_pathWalk.length() - 1; i++) {
        Vector from = graph[m_pathWalk.at(i)].origin;
        Vector to = graph[m_pathWalk.at(i + 1)].origin;

        // Draw line between nodes
        game.drawLine(graph.getEditor(), from, to,
                      5, 0, Color(0, 255, 0), 255, 0, 10);
    }
}
```

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: Path Types

### File: `inc/graph.h:36-41`

```cpp
CR_DECLARE_SCOPED_ENUM(FindPath,
   Fast = 0,      // Shortest distance
   Optimal,       // Consider danger
   Safe           // Avoid combat areas
)
```

Different path types use different heuristics:
- **Fast**: Pure distance
- **Optimal**: Considers bot preferences
- **Safe**: Avoids dangerous areas (uses practice data)

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Node graph | Waypoints and connections |
| A* algorithm | Optimal pathfinding |
| g(n) | Cost from start to n |
| h(n) | Estimated cost from n to goal |
| f(n) | Total estimated cost: g(n) + h(n) |
| PathWalk | Resulting path storage |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript A* (simplified)
function aStar(start: number, goal: number): number[] {
    const openSet = new PriorityQueue();
    const gScore = new Map<number, number>();
    const cameFrom = new Map<number, number>();

    openSet.add(start, heuristic(start, goal));
    gScore.set(start, 0);

    while (!openSet.isEmpty()) {
        const current = openSet.pop();
        if (current === goal) return reconstructPath(cameFrom, current);

        for (const neighbor of getNeighbors(current)) {
            const tentativeG = gScore.get(current)! + distance(current, neighbor);
            if (tentativeG < (gScore.get(neighbor) ?? Infinity)) {
                cameFrom.set(neighbor, current);
                gScore.set(neighbor, tentativeG);
                openSet.add(neighbor, tentativeG + heuristic(neighbor, goal));
            }
        }
    }
    return [];  // No path
}
```

## What's Next

In Lesson 14, you'll learn about Task-Based AI:

- State machines and task priorities
- How bots decide what to do
- The desire/priority system

**[Next: Lesson 14 - Task-Based AI](../14-task-ai/README.md)**
