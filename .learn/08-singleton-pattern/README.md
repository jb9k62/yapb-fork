# Lesson 08: Singleton Pattern

Learn how YaPB uses the Singleton pattern to manage global subsystems like the bot manager, navigation graph, and game state.

## Learning Objectives

- Understand the Singleton pattern
- Learn `CR_EXPOSE_GLOBAL_SINGLETON` macro
- Use YaPB's global instances (bots, graph, game)
- Write a function using multiple singletons

## Theory: What is a Singleton?

A Singleton ensures:
1. Only one instance of a class exists
2. Global access to that instance

### TypeScript Singleton

```typescript
class BotManager {
    private static instance: BotManager;

    private constructor() {}  // Private constructor

    static getInstance(): BotManager {
        if (!BotManager.instance) {
            BotManager.instance = new BotManager();
        }
        return BotManager.instance;
    }
}

// Usage
const bots = BotManager.getInstance();
```

### C++ Singleton (Basic)

```cpp
class BotManager {
private:
    static BotManager* s_instance;
    BotManager() {}  // Private constructor

public:
    static BotManager& instance() {
        if (!s_instance) {
            s_instance = new BotManager();
        }
        return *s_instance;
    }
};

// Usage
BotManager& bots = BotManager::instance();
```

## Theory: YaPB's Singleton Implementation

YaPB uses a cleaner approach with templates and macros.

### File: `ext/crlib/crlib/basic.h` (simplified)

```cpp
template <typename T>
class Singleton {
protected:
    Singleton() = default;

public:
    static T& instance() {
        static T inst;  // Created once, lives forever
        return inst;
    }
};
```

### Inheriting from Singleton

```cpp
class BotManager final : public Singleton<BotManager> {
    // ...
};

// Usage
BotManager::instance().addBot();
```

### The Global Access Macro

```cpp
// Macro creates a global reference
#define CR_EXPOSE_GLOBAL_SINGLETON(type, name) \
    static type &name = type::instance()

// Usage
CR_EXPOSE_GLOBAL_SINGLETON(BotManager, bots);

// Now you can use:
bots.addBot();  // Instead of BotManager::instance().addBot()
```

## Code Study: YaPB's Singletons

### File: `inc/graph.h:390`

```cpp
// Graph class definition
class BotGraph final : public Singleton<BotGraph> {
    // ...
};

// Global access
CR_EXPOSE_GLOBAL_SINGLETON(BotGraph, graph);
```

### File: `inc/engine.h:152-534`

```cpp
class Game final : public Singleton<Game> {
    // ...
};

CR_EXPOSE_GLOBAL_SINGLETON(Game, game);
```

### File: `inc/manager.h` (around line 170)

```cpp
class BotManager final : public Singleton<BotManager> {
    // ...
};

CR_EXPOSE_GLOBAL_SINGLETON(BotManager, bots);
```

## YaPB's Main Singletons

| Global | Class | Purpose |
|--------|-------|---------|
| `game` | `Game` | Engine/game state, map info |
| `bots` | `BotManager` | Bot creation, management |
| `graph` | `BotGraph` | Navigation graph (nodes) |
| `planner` | `PathPlanner` | A* pathfinding |
| `conf` | `BotConfig` | Configuration loading |
| `ctrl` | `BotControl` | Console commands |

### Using Singletons

```cpp
// Get current map name
const char* map = game.getMapName();

// Get number of bots
int count = bots.getBotCount();

// Get number of navigation nodes
int nodes = graph.length();

// Send a message to all players
ctrl.msg("Hello from YaPB!");
```

## Exercise: Write a Multi-Singleton Status Function

Create a function that gathers info from multiple singletons and prints a summary.

### Your Task

1. Create a `printGameStatus()` function
2. Use `game`, `bots`, and `graph` singletons
3. Print a comprehensive status summary

### Implementation

Add to `src/botlib.cpp`:

```cpp
void printGameStatus() {
    // From 'game' singleton
    const char* mapName = game.getMapName();
    bool isDedicated = game.isDedicated();
    float gameTime = game.time();

    // From 'bots' singleton
    int botCount = bots.getBotCount();
    int aliveCount = 0;
    bots.forEach([&aliveCount](Bot* bot) {
        if (bot->m_isAlive) aliveCount++;
        return true;
    });

    // From 'graph' singleton
    int nodeCount = graph.length();
    bool hasGraph = nodeCount > 0;

    // Print summary
    ctrl.msg("=== YaPB Status ===");
    ctrl.msg("Map: %s", mapName);
    ctrl.msg("Time: %.1f seconds", gameTime);
    ctrl.msg("Server: %s", isDedicated ? "Dedicated" : "Listen");
    ctrl.msg("Bots: %d total, %d alive", botCount, aliveCount);
    ctrl.msg("Graph: %s (%d nodes)",
             hasGraph ? "Loaded" : "Not loaded",
             nodeCount);
}
```

### Add a Console Command

In `src/control.cpp`, find the command handler and add:

```cpp
if (isCommand("gamestatus")) {
    printGameStatus();
    return true;
}
```

### Build and Test

```bash
meson compile -C build
./deploy.sh
```

In game:
```
yb gamestatus
```

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: Why Singletons?

### Benefits in Game Development

1. **Global State**: Game has one map, one physics, one time
2. **Cross-System Communication**: Bot needs graph, graph needs game
3. **Performance**: No pointer chasing, direct access

### Potential Downsides

1. **Hidden Dependencies**: Not clear what a function uses
2. **Testing**: Hard to mock singletons
3. **Threading**: Must be careful with concurrent access

### YaPB's Mitigation

- Uses `mutable Mutex` for thread safety in hot paths
- Singletons are well-documented
- Prefer passing parameters for small functions

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Singleton | Single instance, global access |
| `Singleton<T>` | Template base class |
| `CR_EXPOSE_GLOBAL_SINGLETON` | Creates global reference |
| `game` | Game state singleton |
| `bots` | Bot manager singleton |
| `graph` | Navigation graph singleton |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript singleton
class BotManager {
    private static _instance: BotManager;
    static get instance() {
        return this._instance ??= new BotManager();
    }
}
const bots = BotManager.instance;

// C++ singleton
class BotManager : public Singleton<BotManager> {};
CR_EXPOSE_GLOBAL_SINGLETON(BotManager, bots);
```

```typescript
// TypeScript usage
const count = BotManager.instance.getBotCount();

// C++ usage (with global)
const int count = bots.getBotCount();
```

## Common Mistakes

### Accessing Before Initialization

```cpp
// Danger: Singleton might not be ready
int early = bots.getBotCount();  // Called too early?

// YaPB ensures singletons init in correct order
// via static initialization order
```

### Forgetting the Reference

```cpp
// Wrong: Creates a COPY (if allowed)
BotManager copy = BotManager::instance();

// Correct: Uses reference
BotManager& ref = BotManager::instance();

// With global, this is automatic
bots.addBot();  // bots is already a reference
```

## What's Next

In Lesson 09, you'll learn about inheritance and virtual functions:

- Why YaPB uses `final` everywhere
- The `NonCopyable` base class
- When inheritance is (and isn't) useful

**[Next: Lesson 09 - Inheritance & Virtual Functions](../09-inheritance-virtual/README.md)**
