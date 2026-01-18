// Solution for Lesson 03: Functions & Control Flow
//
// This exercise adds a debug function to print enemy count.

// ============================================================
// STEP 1: Add declaration in inc/yapb.h
// ============================================================
// Find the public methods section of the Bot class (around line 760)
// Add this line among the other method declarations:

void debugEnemyCount();

// ============================================================
// STEP 2: Add definition in src/combat.cpp
// ============================================================
// Add this function anywhere in the file (after the includes):

void Bot::debugEnemyCount() {
    // Count enemies within 1000 units (game units, roughly 1000 inches)
    int enemies = numEnemiesNear(pev->origin, 1000.0f);

    // Only print if there are enemies (to avoid spam)
    if (enemies > 0) {
        // ctrl.msg() sends a message to all clients
        // pev->netname.chars() gets the bot's name as a C string
        ctrl.msg("Bot %s sees %d enemies nearby!",
                 pev->netname.chars(), enemies);
    }
}

// ============================================================
// STEP 3: Call from normal_() in src/tasks.cpp
// ============================================================
// Find the normal_() function and add the call at the beginning:

void Bot::normal_() {
    // Debug: Print enemy count
    debugEnemyCount();

    // ... rest of existing code stays the same
    m_aimFlags |= AimFlags::Nav;
    // ... etc
}

// ============================================================
// ALTERNATIVE: Throttled version (better performance)
// ============================================================
// The above will print every frame. Here's a throttled version:

void Bot::debugEnemyCount() {
    // Only run once per second
    static float nextCheck = 0.0f;
    if (game.time() < nextCheck) {
        return;
    }
    nextCheck = game.time() + 1.0f;

    int enemies = numEnemiesNear(pev->origin, 1000.0f);

    if (enemies > 0) {
        ctrl.msg("Bot %s sees %d enemies nearby!",
                 pev->netname.chars(), enemies);
    }
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class Bot {
    private lastCheckTime = 0;

    debugEnemyCount(): void {
        // Throttle to once per second
        const now = Date.now();
        if (now - this.lastCheckTime < 1000) {
            return;
        }
        this.lastCheckTime = now;

        const enemies = this.numEnemiesNear(this.position, 1000);

        if (enemies > 0) {
            console.log(`Bot ${this.name} sees ${enemies} enemies nearby!`);
        }
    }

    numEnemiesNear(origin: Vector, radius: number): number {
        let count = 0;
        for (const client of this.getClients()) {
            if (!client.isUsed || !client.isAlive || client.team === this.team) {
                continue;
            }
            if (client.origin.distanceTo(origin) < radius) {
                count++;
            }
        }
        return count;
    }
}
*/

// ============================================================
// KEY CONCEPTS DEMONSTRATED
// ============================================================

// 1. Method declaration vs definition:
//    - Declaration in header: void debugEnemyCount();
//    - Definition in source: void Bot::debugEnemyCount() { ... }

// 2. Calling another method:
//    - numEnemiesNear() is called without 'this->'
//    - In C++, 'this->' is optional for member access

// 3. Using pev (player entity variables):
//    - pev->origin is the bot's position
//    - pev->netname is the bot's name

// 4. String formatting:
//    - ctrl.msg() works like printf()
//    - %s = string, %d = decimal integer

// 5. Static local variables for throttling:
//    - 'static float nextCheck' persists between calls
//    - Different from TypeScript where you'd use instance variable

// 6. game.time() returns current game time in seconds:
//    - Similar to Date.now() / 1000 in TypeScript
