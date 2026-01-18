// Solution for Lesson 04: Structs & Data Organization
//
// This exercise creates a BotStats struct to track performance.

// ============================================================
// STEP 1: Define BotStats struct in inc/yapb.h
// ============================================================
// Add this before the Bot class definition (around line 140):

struct BotStats {
    int32_t kills {};           // number of kills
    int32_t deaths {};          // number of deaths
    int32_t headshots {};       // headshot kills
    float damageDealt {};       // total damage dealt
    float damageTaken {};       // total damage taken
    float playTime {};          // seconds of game time

    // Calculate kill/death ratio
    float getKDRatio() const {
        if (deaths == 0) {
            return static_cast<float>(kills);
        }
        return static_cast<float>(kills) / static_cast<float>(deaths);
    }

    // Calculate headshot percentage
    float getHeadshotPercent() const {
        if (kills == 0) {
            return 0.0f;
        }
        return 100.0f * static_cast<float>(headshots) / static_cast<float>(kills);
    }

    // Reset all stats
    void reset() {
        kills = 0;
        deaths = 0;
        headshots = 0;
        damageDealt = 0.0f;
        damageTaken = 0.0f;
        playTime = 0.0f;
    }
};

// ============================================================
// STEP 2: Add member to Bot class in inc/yapb.h
// ============================================================
// In the Bot class public section (around line 720), add:

public:
    BotStats m_stats {};  // performance statistics

// ============================================================
// STEP 3: Add printStats declaration in inc/yapb.h
// ============================================================
// In the Bot class public methods section, add:

    void printStats();

// ============================================================
// STEP 4: Implement printStats in src/combat.cpp
// ============================================================

void Bot::printStats() {
    ctrl.msg("%s Stats - K/D: %d/%d (%.2f), HS: %d (%.1f%%), DMG: %.0f/%.0f",
             pev->netname.chars(),      // Bot name
             m_stats.kills,             // Kill count
             m_stats.deaths,            // Death count
             m_stats.getKDRatio(),      // K/D ratio
             m_stats.headshots,         // Headshot count
             m_stats.getHeadshotPercent(), // HS percentage
             m_stats.damageDealt,       // Damage output
             m_stats.damageTaken);      // Damage received
}

// ============================================================
// OPTIONAL: Hook into kill/death events
// ============================================================
// To actually update stats, you'd modify these functions:

// In src/manager.cpp, handleDeath() function:
void BotManager::handleDeath(edict_t *killer, edict_t *victim) {
    // ... existing code ...

    Bot *killerBot = findBotByEntity(killer);
    Bot *victimBot = findBotByEntity(victim);

    if (killerBot) {
        killerBot->m_stats.kills++;
        // Check if headshot by damage type
    }

    if (victimBot) {
        victimBot->m_stats.deaths++;
    }
}

// In src/botlib.cpp, takeDamage() function:
void Bot::takeDamage(edict_t *inflictor, int damage, int armor, int bits) {
    // ... existing code ...

    m_stats.damageTaken += static_cast<float>(damage);

    // If we know the attacker is a bot, update their stats
    Bot *attacker = bots[inflictor];
    if (attacker) {
        attacker->m_stats.damageDealt += static_cast<float>(damage);
    }
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
interface BotStats {
    kills: number;
    deaths: number;
    headshots: number;
    damageDealt: number;
    damageTaken: number;
    playTime: number;
}

class BotStatsImpl implements BotStats {
    kills = 0;
    deaths = 0;
    headshots = 0;
    damageDealt = 0;
    damageTaken = 0;
    playTime = 0;

    getKDRatio(): number {
        return this.deaths === 0 ? this.kills : this.kills / this.deaths;
    }

    getHeadshotPercent(): number {
        return this.kills === 0 ? 0 : (100 * this.headshots) / this.kills;
    }

    reset(): void {
        this.kills = 0;
        this.deaths = 0;
        this.headshots = 0;
        this.damageDealt = 0;
        this.damageTaken = 0;
        this.playTime = 0;
    }
}

class Bot {
    stats: BotStats = new BotStatsImpl();

    printStats(): void {
        console.log(
            `${this.name} Stats - K/D: ${this.stats.kills}/${this.stats.deaths} ` +
            `(${this.stats.getKDRatio().toFixed(2)}), ` +
            `HS: ${this.stats.headshots} (${this.stats.getHeadshotPercent().toFixed(1)}%)`
        );
    }
}
*/

// ============================================================
// MEMORY LAYOUT ANALYSIS
// ============================================================

// BotStats memory layout:
// Offset 0:  int32_t kills       (4 bytes)
// Offset 4:  int32_t deaths      (4 bytes)
// Offset 8:  int32_t headshots   (4 bytes)
// Offset 12: float damageDealt   (4 bytes)
// Offset 16: float damageTaken   (4 bytes)
// Offset 20: float playTime      (4 bytes)
// Total: 24 bytes, no padding needed (all 4-byte aligned)

// This is an efficient layout because:
// 1. All members are 4-byte aligned naturally
// 2. No padding between members
// 3. Grouped by type (ints together, floats together)

// If we had ordered it poorly:
// struct BadLayout {
//     bool active;        // 1 byte + 3 padding
//     int32_t kills;      // 4 bytes
//     bool ranked;        // 1 byte + 3 padding
//     float damage;       // 4 bytes
// };
// Total: 16 bytes (4 bytes wasted on padding)
