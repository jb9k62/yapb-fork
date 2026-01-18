// Solution for Lesson 08: Singleton Pattern
//
// This exercise creates a function using multiple singletons.

// ============================================================
// Add to src/botlib.cpp (or create a new file)
// ============================================================

void printGameStatus() {
    ctrl.msg("=== YaPB Game Status ===");

    // ----------------------------------------
    // Information from 'game' singleton
    // ----------------------------------------
    ctrl.msg("--- Game Info ---");
    ctrl.msg("  Map: %s", game.getMapName());
    ctrl.msg("  Time: %.1f seconds", game.time());
    ctrl.msg("  Server: %s", game.isDedicated() ? "Dedicated" : "Listen");

    // Check map type
    if (game.is(MapFlags::Demolition)) {
        ctrl.msg("  Type: Bomb defusal (de_)");
    } else if (game.is(MapFlags::HostageRescue)) {
        ctrl.msg("  Type: Hostage rescue (cs_)");
    } else if (game.is(MapFlags::Assassination)) {
        ctrl.msg("  Type: VIP escort (as_)");
    } else {
        ctrl.msg("  Type: Other");
    }

    // ----------------------------------------
    // Information from 'bots' singleton
    // ----------------------------------------
    ctrl.msg("--- Bot Info ---");

    int botCount = bots.getBotCount();
    int aliveT = 0, aliveCT = 0, deadCount = 0;

    bots.forEach([&](Bot* bot) {
        if (bot->m_isAlive) {
            if (bot->m_team == Team::Terrorist) {
                aliveT++;
            } else {
                aliveCT++;
            }
        } else {
            deadCount++;
        }
        return true;  // Continue iteration
    });

    ctrl.msg("  Total Bots: %d", botCount);
    ctrl.msg("  Terrorists Alive: %d", aliveT);
    ctrl.msg("  Counter-Terrorists Alive: %d", aliveCT);
    ctrl.msg("  Dead: %d", deadCount);

    // ----------------------------------------
    // Information from 'graph' singleton
    // ----------------------------------------
    ctrl.msg("--- Navigation Graph ---");

    int nodeCount = graph.length();
    if (nodeCount > 0) {
        ctrl.msg("  Status: Loaded");
        ctrl.msg("  Nodes: %d", nodeCount);
        ctrl.msg("  Author: %s",
                 graph.getAuthor().empty() ? "Unknown" : graph.getAuthor().chars());
    } else {
        ctrl.msg("  Status: NOT LOADED");
        ctrl.msg("  Note: Bots cannot navigate without a graph!");
    }

    ctrl.msg("========================");
}

// ============================================================
// Add console command in src/control.cpp
// ============================================================
// Find the command handler function and add:

if (isCommand("gamestatus") || isCommand("status")) {
    printGameStatus();
    return true;
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
function printGameStatus(): void {
    console.log("=== YaPB Game Status ===");

    // Using singleton instances (TypeScript style)
    const game = Game.instance;
    const bots = BotManager.instance;
    const graph = BotGraph.instance;

    console.log("--- Game Info ---");
    console.log(`  Map: ${game.getMapName()}`);
    console.log(`  Time: ${game.time().toFixed(1)} seconds`);

    console.log("--- Bot Info ---");
    console.log(`  Total Bots: ${bots.getBotCount()}`);

    let aliveCount = 0;
    bots.forEach(bot => {
        if (bot.isAlive) aliveCount++;
        return true;
    });
    console.log(`  Alive: ${aliveCount}`);

    console.log("--- Navigation Graph ---");
    console.log(`  Nodes: ${graph.length()}`);
}
*/

// ============================================================
// KEY CONCEPTS DEMONSTRATED
// ============================================================

// 1. Multiple singleton access:
//    game.getMapName()  - Game singleton
//    bots.getBotCount() - BotManager singleton
//    graph.length()     - BotGraph singleton

// 2. Lambda with capture:
//    bots.forEach([&](Bot* bot) { ... })
//    The [&] captures local variables by reference

// 3. Checking game flags:
//    game.is(MapFlags::Demolition)
//    Uses bitwise AND internally to check flags

// 4. Conditional string:
//    graph.getAuthor().empty() ? "Unknown" : graph.getAuthor().chars()
//    Ternary operator for null-safe string output

// ============================================================
// ADVANCED: Timing multiple singleton calls
// ============================================================

void printTimedStatus() {
    float start = game.time();

    // Do expensive operations
    int nodeCount = graph.length();
    int botCount = bots.getBotCount();

    float elapsed = game.time() - start;
    ctrl.msg("Status gathered in %.3f ms", elapsed * 1000.0f);
}

// ============================================================
// PATTERN: Coordinated multi-singleton operation
// ============================================================

void notifyAllBotsOfGraphChange() {
    // When graph changes, notify all bots

    if (graph.length() == 0) {
        ctrl.msg("Warning: Graph unloaded!");

        // Tell all bots to stop pathfinding
        bots.forEach([](Bot* bot) {
            bot->clearSearchNodes();
            return true;
        });
    }
}

// ============================================================
// The singletons work together:
// ============================================================
//
// game   ─── Provides: time, map info, entity access
//    │
//    └─── Used by: bots (for timing), graph (for map data)
//
// bots   ─── Provides: bot creation, iteration, management
//    │
//    └─── Uses: game (entity creation), graph (navigation)
//
// graph  ─── Provides: navigation nodes, pathfinding
//    │
//    └─── Uses: game (map geometry for analysis)
//
// This is why singletons are useful - they represent
// truly global game systems that need to interact.
