// Solution for Lesson 15: Performance & Optimization
//
// Frame budget monitoring and performance tracking.

// ============================================================
// FRAME BUDGET MONITOR
// ============================================================

class FrameProfiler {
    float m_frameStart {};
    float m_maxTime {};
    float m_totalTime {};
    int m_frameCount {};

    // Per-operation tracking
    float m_pathTime {};
    float m_combatTime {};
    float m_navTime {};
    float m_otherTime {};

public:
    void beginFrame() {
        m_frameStart = game.time();
    }

    void recordOperation(const char* name, float duration) {
        if (strcmp(name, "path") == 0) m_pathTime += duration;
        else if (strcmp(name, "combat") == 0) m_combatTime += duration;
        else if (strcmp(name, "nav") == 0) m_navTime += duration;
        else m_otherTime += duration;
    }

    void endFrame() {
        float frameTime = (game.time() - m_frameStart) * 1000.0f;

        m_maxTime = cr::max(m_maxTime, frameTime);
        m_totalTime += frameTime;
        m_frameCount++;

        // Warn on slow frames
        if (frameTime > 5.0f) {
            ctrl.msg("SLOW FRAME: %.2f ms (path:%.2f, combat:%.2f, nav:%.2f)",
                     frameTime,
                     m_pathTime * 1000.0f,
                     m_combatTime * 1000.0f,
                     m_navTime * 1000.0f);
        }

        // Reset per-frame counters
        m_pathTime = m_combatTime = m_navTime = m_otherTime = 0;

        // Report every 500 frames
        if (m_frameCount >= 500) {
            float avgTime = m_totalTime / m_frameCount;
            ctrl.msg("Bot AI Stats: avg=%.2fms, max=%.2fms (%d bots)",
                     avgTime, m_maxTime, bots.getBotCount());

            m_maxTime = 0;
            m_totalTime = 0;
            m_frameCount = 0;
        }
    }
};

static FrameProfiler profiler;

// ============================================================
// MODIFIED BotManager::frame() with profiling
// ============================================================

void BotManager::frame() {
    profiler.beginFrame();

    for (auto& bot : m_bots) {
        if (!bot || bot->m_isStale) continue;

        // Profile navigation
        float start = game.time();
        bot->updateNavigation();
        profiler.recordOperation("nav", game.time() - start);

        // Profile combat
        start = game.time();
        bot->updateCombat();
        profiler.recordOperation("combat", game.time() - start);

        // Profile pathfinding (when it occurs)
        start = game.time();
        bot->frame();
        profiler.recordOperation("other", game.time() - start);
    }

    profiler.endFrame();
}

// ============================================================
// OPTIMIZED ENEMY SCANNING
// ============================================================

bool Bot::scanForEnemiesOptimized() {
    // Early exit checks
    if (cv_ignore_enemies) return false;
    if (game.time() < m_enemyUpdateTime) return m_enemy != nullptr;

    m_enemyUpdateTime = game.time() + 0.1f;  // Throttle to 10Hz

    // Quick check: do we have a valid enemy?
    if (!game.isNullEntity(m_enemy)) {
        // Verify still valid
        if (m_enemy->v.health > 0 && seesEnemy(m_enemy)) {
            return true;  // Keep current enemy
        }
        m_enemy = nullptr;  // Lost enemy
    }

    // Full scan with early termination
    float bestDistSq = kInfiniteDistance * kInfiniteDistance;
    edict_t* bestEnemy = nullptr;

    for (const auto& client : util.getClients()) {
        // Quick rejection checks (ordered by cost)
        if (!(client.flags & ClientFlags::Used)) continue;
        if (!(client.flags & ClientFlags::Alive)) continue;
        if (client.team == m_team) continue;

        edict_t* ent = client.ent;
        if (game.isNullEntity(ent)) continue;

        // Distance check (cheap)
        float distSq = pev->origin.distanceSq(client.origin);
        if (distSq > m_maxViewDistance * m_maxViewDistance) continue;
        if (distSq > bestDistSq) continue;  // Already have closer

        // Visibility check (expensive - do last)
        if (!seesEnemy(ent)) continue;

        bestDistSq = distSq;
        bestEnemy = ent;
    }

    m_enemy = bestEnemy;
    return m_enemy != nullptr;
}

// ============================================================
// THROTTLED OPERATIONS HELPER
// ============================================================

class ThrottledTimer {
    float m_nextTime {};
    float m_interval;

public:
    ThrottledTimer(float interval) : m_interval(interval) {}

    bool check() {
        float now = game.time();
        if (now < m_nextTime) return false;
        m_nextTime = now + m_interval;
        return true;
    }
};

// Usage:
// ThrottledTimer pickupTimer(0.5f);  // Every 500ms
// if (pickupTimer.check()) {
//     updatePickups();  // Expensive
// }

// ============================================================
// CONSOLE COMMAND: Performance stats
// ============================================================

// In src/control.cpp:
if (isCommand("perf") || isCommand("performance")) {
    ctrl.msg("=== Bot AI Performance ===");
    ctrl.msg("Bots: %d", bots.getBotCount());
    ctrl.msg("Frame rate: ~%.0f FPS", 1.0f / game.getFrameTime());

    // Per-bot stats
    float totalTime = 0;
    bots.forEach([&](Bot* bot) {
        // Would need timing instrumentation
        return true;
    });

    return true;
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class FrameProfiler {
    private frameStart = 0;
    private maxTime = 0;
    private totalTime = 0;
    private frameCount = 0;

    beginFrame(): void {
        this.frameStart = performance.now();
    }

    endFrame(): void {
        const frameTime = performance.now() - this.frameStart;
        this.maxTime = Math.max(this.maxTime, frameTime);
        this.totalTime += frameTime;
        this.frameCount++;

        if (frameTime > 5) {
            console.warn(`Slow frame: ${frameTime.toFixed(2)}ms`);
        }

        if (this.frameCount >= 500) {
            const avg = this.totalTime / this.frameCount;
            console.log(`Bot AI: avg=${avg.toFixed(2)}ms, max=${this.maxTime.toFixed(2)}ms`);
            this.maxTime = 0;
            this.totalTime = 0;
            this.frameCount = 0;
        }
    }
}

class ThrottledTimer {
    private nextTime = 0;
    constructor(private interval: number) {}

    check(): boolean {
        const now = Date.now();
        if (now < this.nextTime) return false;
        this.nextTime = now + this.interval;
        return true;
    }
}
*/

// ============================================================
// PERFORMANCE OPTIMIZATION CHECKLIST
// ============================================================

/*
□ Expensive operations throttled (canRunHeavyWeight)
□ Using distanceSq instead of distance
□ Early exits in scanning loops
□ Cached calculations reused
□ Limits on iteration counts
□ Time-based operation spacing
□ Profiling shows <5ms per frame
□ No frame spikes >10ms
*/
