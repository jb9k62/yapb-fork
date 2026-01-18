# Lesson 15: Performance & Optimization

Learn to keep bot AI fast enough for smooth gameplay with many bots.

## Learning Objectives

- Understand frame budgeting
- Learn throttling patterns
- Profile bot code for bottlenecks
- Implement frame-budgeted operations

## Theory: The Performance Challenge

With 10 bots at 100 FPS:
- 1000 bot-frames per second
- Each bot-frame must complete in ~1ms
- Total bot AI budget: ~10ms per frame

If bot AI takes too long → game stutters.

### Frame Budget

```
At 100 FPS: 10ms total frame time
├── Engine physics: ~3ms
├── Rendering: ~4ms
├── Bot AI: ~2ms ← Our budget
└── Other: ~1ms

If bot AI exceeds 5ms, players notice stuttering.
```

## Theory: Expensive Operations

### CPU Cost Hierarchy

| Operation | Cost | Frequency |
|-----------|------|-----------|
| Math (+, -, *) | Very cheap | Every frame |
| Array access | Cheap | Every frame |
| Function call | Cheap | Every frame |
| sqrt/sin/cos | Moderate | Throttle |
| Pathfinding | Expensive | Heavily throttle |
| Entity iteration | Moderate | Throttle |
| Visibility traces | Expensive | Throttle |

### Common Bottlenecks in Bot AI

1. **Pathfinding** - A* is O(n log n), expensive
2. **Enemy scanning** - Checking all players
3. **Visibility checks** - Raytracing to enemies
4. **Navigation** - Finding nearest nodes

## Code Study: canRunHeavyWeight

### File: `src/navigate.cpp`

```cpp
bool Bot::canRunHeavyWeight() {
    // Only one bot runs expensive operations per frame
    // Spreads CPU load across multiple frames

    static int32_t currentSlot = 0;
    int32_t totalBots = bots.getBotCount();

    if (totalBots == 0) {
        return true;
    }

    // Round-robin: each bot gets a turn
    bool canRun = (m_index == currentSlot);

    // Move to next slot
    currentSlot = (currentSlot + 1) % totalBots;

    return canRun;
}
```

### Usage Pattern

```cpp
void Bot::frame() {
    // Always run (cheap)
    updateAiming();
    checkReload();

    // Only run when allowed (expensive)
    if (canRunHeavyWeight()) {
        scanForEnemies();     // Expensive
        updatePickups();      // Moderate
        recalculatePath();    // Very expensive
    }
}
```

## Theory: Time-Based Throttling

### Pattern: Periodic Execution

```cpp
void Bot::updatePickups() {
    // Only run every 0.5 seconds
    if (game.time() < m_itemCheckTime) {
        return;
    }
    m_itemCheckTime = game.time() + 0.5f;

    // Expensive pickup scanning...
}
```

### Pattern: Staggered Updates

```cpp
void Bot::logic() {
    // Different operations on different intervals
    float time = game.time();

    // Every frame
    updateMovement();

    // Every 100ms
    if (time > m_nextEnemyCheck) {
        checkForEnemies();
        m_nextEnemyCheck = time + 0.1f;
    }

    // Every 500ms
    if (time > m_nextPathCheck) {
        validatePath();
        m_nextPathCheck = time + 0.5f;
    }
}
```

## Theory: Squared Distance

Avoid `sqrt()` when comparing distances:

```cpp
// Slow: Uses sqrt twice
float dist1 = (a - enemy1->origin).length();
float dist2 = (a - enemy2->origin).length();
bool closer = dist1 < dist2;

// Fast: No sqrt
float distSq1 = (a - enemy1->origin).lengthSq();
float distSq2 = (a - enemy2->origin).lengthSq();
bool closer = distSq1 < distSq2;

// For radius checks
float radiusSq = radius * radius;  // Compute once
if (distSq < radiusSq) { ... }     // Compare squared
```

## Exercise: Implement Frame Budget Monitor

Create a system to monitor and warn about slow frames.

### Your Task

1. Track time spent in bot AI
2. Warn when budget exceeded
3. Track which operations are slow

### Implementation

```cpp
// In BotManager::frame()
void BotManager::frame() {
    float startTime = game.time();

    // Track per-operation timing
    float pathTime = 0, combatTime = 0, navTime = 0;

    for (auto& bot : m_bots) {
        if (!bot || bot->m_isStale) continue;

        float opStart = game.time();
        bot->updateNavigation();
        navTime += game.time() - opStart;

        opStart = game.time();
        bot->updateCombat();
        combatTime += game.time() - opStart;
    }

    float totalTime = (game.time() - startTime) * 1000.0f;

    // Warning thresholds
    if (totalTime > 5.0f) {
        ctrl.msg("WARNING: Bot AI frame took %.2f ms!", totalTime);
        ctrl.msg("  Navigation: %.2f ms", navTime * 1000);
        ctrl.msg("  Combat: %.2f ms", combatTime * 1000);
    }

    // Track statistics
    static float maxTime = 0;
    static float avgTime = 0;
    static int samples = 0;

    maxTime = cr::max(maxTime, totalTime);
    avgTime = (avgTime * samples + totalTime) / (samples + 1);
    samples++;

    // Report every 500 frames
    if (samples >= 500) {
        ctrl.msg("Bot AI: avg=%.2fms, max=%.2fms (%d bots)",
                 avgTime, maxTime, getBotCount());
        maxTime = 0;
        avgTime = 0;
        samples = 0;
    }
}
```

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: Early Exit Patterns

### Skip Unnecessary Work

```cpp
void Bot::checkForEnemies() {
    // Early exit: no enemies possible
    if (game.is(GameFlags::FreeForAll) && getAliveCount() <= 1) {
        return;
    }

    // Early exit: already have valid enemy
    if (!game.isNullEntity(m_enemy) && m_enemy->v.health > 0) {
        if (game.time() < m_enemyUpdateTime) {
            return;  // Enemy still valid, skip expensive scan
        }
    }

    // Full scan (expensive)
    for (const auto& client : util.getClients()) {
        // ...
    }
}
```

### Limit Iterations

```cpp
void Bot::findNearestEnemy() {
    int checked = 0;
    const int maxChecks = 16;  // Don't check all 32 players

    for (const auto& client : util.getClients()) {
        if (checked++ >= maxChecks) break;
        // ...
    }
}
```

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Frame budget | Time allowed for bot AI per frame |
| canRunHeavyWeight | Round-robin expensive operations |
| Time throttling | Run operations periodically |
| distanceSq | Avoid sqrt for comparisons |
| Early exit | Skip work when possible |
| Profiling | Measure actual performance |

## Performance Tips Summary

1. **Throttle expensive operations** - Don't do everything every frame
2. **Use squared distances** - Avoid sqrt when possible
3. **Early exit** - Skip work that isn't needed
4. **Spread load** - Use canRunHeavyWeight pattern
5. **Profile** - Measure before optimizing
6. **Cache results** - Store computed values
7. **Limit iterations** - Don't check all entities

## Congratulations!

You've completed the YaPB C++ Learning Curriculum!

### What You've Learned

- C++ fundamentals from a TypeScript perspective
- Memory management (pointers, RAII, smart pointers)
- Object-oriented programming patterns
- GoldSrc engine integration
- Game loop and frame-based execution
- Entity systems and vector math
- A* pathfinding and task-based AI
- Performance optimization techniques

### Next Steps

1. **Explore the codebase** - Read more of YaPB's source
2. **Make modifications** - Add features or fix bugs
3. **Create custom behaviors** - Add new bot personalities
4. **Contribute** - Submit improvements to the project

**Happy coding!**
