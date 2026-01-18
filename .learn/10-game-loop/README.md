# Lesson 10: The Game Loop

Understand frame-based execution and how YaPB's bot AI runs every game frame.

## Learning Objectives

- Understand frame-based game execution
- Learn about the `StartFrame` callback
- Trace how bots think every frame
- Add frame timing debug output

## Theory: Game Loop Basics

### TypeScript Event-Driven Model

```typescript
// Node.js/Browser: Event-driven, reactive
document.addEventListener('click', handleClick);
setInterval(update, 16);  // ~60fps with setTimeout

// Async/await for sequences
async function gameLoop() {
    while (running) {
        await update();
        await render();
    }
}
```

### C++ Game Loop Model

```cpp
// Traditional game loop (in engine):
while (running) {
    float deltaTime = getDeltaTime();
    processInput();
    update(deltaTime);
    render();
    // Repeats ~60-100+ times per second
}
```

In GoldSrc, the engine runs this loop and calls **callbacks** in your plugin.

## Theory: GoldSrc Frame Callbacks

The engine calls specific functions in your plugin:

```
Engine Game Loop
    │
    ├── StartFrame()      ← Called every frame
    │   └── Bot::frame()  ← YaPB hooks here
    │
    ├── ClientCommand()   ← Player typed command
    │
    ├── PlayerPreThink()  ← Before player physics
    │
    └── PlayerPostThink() ← After player physics
```

### StartFrame Timing

```
Time ──────────────────────────────────────────────►
     │  Frame 1  │  Frame 2  │  Frame 3  │
     │  10ms     │  10ms     │  10ms     │
     │           │           │           │
     StartFrame  StartFrame  StartFrame
     ↓           ↓           ↓
     Bot thinks  Bot thinks  Bot thinks
```

At 100 FPS, bots think 100 times per second.

## Code Study: StartFrame Callback

### File: `src/linkage.cpp:365-417`

```cpp
table->pfnStartFrame = [] () CR_FORCE_STACK_ALIGN {
   // this function starts a video frame. It is called once per video frame.
   // If you run Half-Life at 90 fps, this function will then be called 90
   // times per second.

   // update lightstyle animations
   illum.animateLight ();

   // update some stats for clients
   util.updateClients ();

   // graph editor frame
   if (graph.hasEditFlag (GraphEdit::On) && graph.hasEditor ()) {
      graph.frame ();
   }

   // update analyzer if needed
   analyzer.update ();

   // run stuff periodically
   game.slowFrame ();

   // rebuild vistable if needed
   vistab.rebuild ();

   if (bots.hasBotsOnline ()) {
      // keep track of grenades on map
      gameState.updateActiveGrenade ();

      // keep track of interesting entities
      gameState.updateInterestingEntities ();
   }

   // keep bot number up to date
   bots.maintainQuota ();

   // balance bot difficulties
   bots.balanceBotDifficulties ();

   // flush print queue to users
   ctrl.flushPrintQueue ();

   // ... metamod handling ...

   // run the bot ai
   bots.frame ();  // <-- THIS IS WHERE BOTS THINK
};
```

### Key Operations Each Frame

1. **Light animation** - Visual updates
2. **Client updates** - Track player states
3. **Quota management** - Add/remove bots as needed
4. **Bot AI** - `bots.frame()` runs all bot logic

## Code Study: Bot Manager Frame

### File: `src/manager.cpp` (BotManager::frame)

```cpp
void BotManager::frame() {
    // Iterate all bots
    for (auto &bot : m_bots) {
        if (bot && !bot->m_isStale) {
            bot->frame();  // Each bot thinks
        }
    }
}
```

## Code Study: Individual Bot Frame

### File: `src/botlib.cpp` (Bot::frame and Bot::logic)

```cpp
void Bot::frame() {
    // Skip if not alive
    if (!m_isAlive) {
        return;
    }

    // Update thinking
    update();

    // Execute current task
    executeTasks();

    // Handle movement
    runMovement();

    // Update look angles
    updateLookAngles();
}
```

### Frame Throttling

Not everything runs every frame:

```cpp
void Bot::logic() {
    // This runs with frame-skipping for expensive operations

    // Heavy operations only run when allowed
    if (canRunHeavyWeight()) {
        checkForEnemies();      // Expensive!
        updatePickups();        // Expensive!
    }

    // Light operations run always
    checkReload();
    updateAiming();
}
```

## Theory: Delta Time

### The Problem

```cpp
// Bad: Frame-rate dependent
void update() {
    position += velocity;  // Moves faster at higher FPS!
}
```

### The Solution

```cpp
// Good: Frame-rate independent
void update(float deltaTime) {
    position += velocity * deltaTime;  // Consistent speed
}
```

### In YaPB

```cpp
// YaPB uses game.time() for timing
float Bot::getConnectionTime() {
    return game.time() - m_joinServerTime;
}

// Timers use game time
if (game.time() > m_nextThinkTime) {
    doExpensiveOperation();
    m_nextThinkTime = game.time() + 0.1f;  // Every 100ms
}
```

## Exercise: Add Frame Timing Debug

Add output that shows frame timing information.

### Your Task

1. Track frame times in StartFrame
2. Calculate and print FPS periodically
3. Show how much time bot AI takes

### Implementation

In `src/linkage.cpp`, modify StartFrame:

```cpp
table->pfnStartFrame = [] () CR_FORCE_STACK_ALIGN {
    // Add at the start:
    static float lastFrameTime = 0.0f;
    static float fpsAccumulator = 0.0f;
    static int frameCount = 0;

    float currentTime = game.time();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    if (deltaTime > 0.0f) {
        fpsAccumulator += 1.0f / deltaTime;
        frameCount++;

        // Print every 100 frames
        if (frameCount >= 100) {
            float avgFps = fpsAccumulator / frameCount;
            ctrl.msg("Average FPS: %.1f", avgFps);
            fpsAccumulator = 0.0f;
            frameCount = 0;
        }
    }

    // Measure bot AI time
    float botStartTime = game.time();

    // ... existing StartFrame code ...
    bots.frame();

    float botEndTime = game.time();
    float botTime = (botEndTime - botStartTime) * 1000.0f;

    // Warn if bot AI takes too long
    if (botTime > 5.0f) {  // More than 5ms
        ctrl.msg("Warning: Bot AI took %.2f ms", botTime);
    }
};
```

### Build and Test

```bash
meson compile -C build
./deploy.sh
```

Watch the console for FPS reports and any warnings.

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: canRunHeavyWeight

### File: `src/navigate.cpp`

```cpp
bool Bot::canRunHeavyWeight() {
    // Spread expensive operations across frames
    // Only one bot runs heavy operations per frame

    static int currentBot = 0;
    currentBot = (currentBot + 1) % bots.getBotCount();

    return m_index == currentBot;
}
```

This ensures expensive operations (pathfinding, enemy scanning) are spread across frames, preventing lag spikes.

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| StartFrame | Engine callback each frame |
| Frame rate | Typically 60-100+ FPS |
| Delta time | Time since last frame |
| Frame throttling | Spread expensive work |
| game.time() | Current game time in seconds |
| bots.frame() | All bots think each frame |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript game loop
setInterval(() => {
    update();
}, 1000 / 60);  // 60 FPS

// C++ callback (GoldSrc)
table->pfnStartFrame = []() {
    update();
};  // Called by engine
```

```typescript
// TypeScript timing
const now = performance.now() / 1000;  // Seconds

// C++ timing (YaPB)
float now = game.time();  // Seconds since map load
```

## Common Mistakes

### Frame-Dependent Logic

```cpp
// Bad: Speed depends on FPS
void move() {
    m_position += m_velocity;  // Faster at higher FPS!
}

// Good: Frame-rate independent
void move(float dt) {
    m_position += m_velocity * dt;
}
```

### Heavy Operations Every Frame

```cpp
// Bad: Expensive every frame
void frame() {
    findAllEnemies();      // O(n) scan
    calculatePath();       // Expensive A*
    updateVisibility();    // Raytracing
}

// Good: Throttled
void frame() {
    if (canRunHeavyWeight()) {
        findAllEnemies();
    }
    if (game.time() > m_nextPathCalc) {
        calculatePath();
        m_nextPathCalc = game.time() + 0.5f;
    }
}
```

## What's Next

In Lesson 11, you'll learn about the Entity System:

- `edict_t` and `entvars_t` structures
- Entity indexing and iteration
- Creating a player listing function

**[Next: Lesson 11 - Entity System](../11-entity-system/README.md)**
