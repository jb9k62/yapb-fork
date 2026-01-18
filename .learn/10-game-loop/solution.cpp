// Solution for Lesson 10: The Game Loop
//
// This exercise adds frame timing debug output.

// ============================================================
// MODIFIED StartFrame in src/linkage.cpp
// ============================================================

table->pfnStartFrame = [] () CR_FORCE_STACK_ALIGN {
    // ========================================
    // TIMING DEBUG CODE (add at start)
    // ========================================
    static float lastFrameTime = 0.0f;
    static float fpsAccumulator = 0.0f;
    static float frameTimeAccum = 0.0f;
    static int frameCount = 0;
    static float nextReportTime = 0.0f;

    float currentTime = game.time();
    float deltaTime = currentTime - lastFrameTime;

    if (lastFrameTime > 0.0f && deltaTime > 0.0f && deltaTime < 1.0f) {
        float fps = 1.0f / deltaTime;
        fpsAccumulator += fps;
        frameTimeAccum += deltaTime * 1000.0f;  // Convert to ms
        frameCount++;
    }
    lastFrameTime = currentTime;

    // Report every 2 seconds
    if (currentTime > nextReportTime && frameCount > 0) {
        float avgFps = fpsAccumulator / frameCount;
        float avgFrameTime = frameTimeAccum / frameCount;

        ctrl.msg("FPS: %.1f (%.2f ms/frame)", avgFps, avgFrameTime);

        fpsAccumulator = 0.0f;
        frameTimeAccum = 0.0f;
        frameCount = 0;
        nextReportTime = currentTime + 2.0f;
    }

    // ========================================
    // ORIGINAL StartFrame code
    // ========================================
    illum.animateLight ();
    util.updateClients ();

    if (graph.hasEditFlag (GraphEdit::On) && graph.hasEditor ()) {
        graph.frame ();
    }

    analyzer.update ();
    game.slowFrame ();
    vistab.rebuild ();

    if (bots.hasBotsOnline ()) {
        gameState.updateActiveGrenade ();
        gameState.updateInterestingEntities ();
    }

    bots.maintainQuota ();
    bots.balanceBotDifficulties ();
    ctrl.flushPrintQueue ();

    // ========================================
    // MEASURE BOT AI TIME
    // ========================================
    float botStartTime = game.time();

    if (game.is (GameFlags::Metamod)) {
        RETURN_META (MRES_IGNORED);
    }
    dllapi.pfnStartFrame ();

    // Bot AI runs here for non-metamod
    bots.frame ();

    float botEndTime = game.time();
    float botTimeMs = (botEndTime - botStartTime) * 1000.0f;

    // Track bot AI time
    static float maxBotTime = 0.0f;
    static float botTimeAccum = 0.0f;
    static int botTimeCount = 0;

    botTimeAccum += botTimeMs;
    botTimeCount++;

    if (botTimeMs > maxBotTime) {
        maxBotTime = botTimeMs;
    }

    // Report bot timing every 5 seconds
    static float nextBotReport = 0.0f;
    if (currentTime > nextBotReport && botTimeCount > 0) {
        float avgBotTime = botTimeAccum / botTimeCount;
        ctrl.msg("Bot AI: avg %.2f ms, max %.2f ms (%d bots)",
                 avgBotTime, maxBotTime, bots.getBotCount());

        botTimeAccum = 0.0f;
        botTimeCount = 0;
        maxBotTime = 0.0f;
        nextBotReport = currentTime + 5.0f;
    }

    // Warn if single frame takes too long
    if (botTimeMs > 10.0f) {
        ctrl.msg("WARNING: Bot AI spike: %.2f ms", botTimeMs);
    }
};

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class GameLoop {
    private lastFrameTime = 0;
    private fpsAccumulator = 0;
    private frameCount = 0;

    frame(): void {
        const currentTime = performance.now() / 1000;
        const deltaTime = currentTime - this.lastFrameTime;
        this.lastFrameTime = currentTime;

        if (deltaTime > 0) {
            this.fpsAccumulator += 1 / deltaTime;
            this.frameCount++;

            if (this.frameCount >= 100) {
                const avgFps = this.fpsAccumulator / this.frameCount;
                console.log(`FPS: ${avgFps.toFixed(1)}`);
                this.fpsAccumulator = 0;
                this.frameCount = 0;
            }
        }

        // Measure bot AI time
        const botStart = performance.now();
        this.bots.frame();
        const botTime = performance.now() - botStart;

        if (botTime > 5) {
            console.warn(`Bot AI spike: ${botTime.toFixed(2)}ms`);
        }
    }
}
*/

// ============================================================
// UNDERSTANDING FRAME TIMING
// ============================================================

/*
At 100 FPS:
- 1 frame = 10ms total
- If bot AI takes 5ms, that's 50% of frame budget!
- If bot AI takes 10ms, game will lag

Frame Budget at Different FPS:
- 60 FPS: 16.67ms per frame
- 100 FPS: 10ms per frame
- 144 FPS: 6.94ms per frame

For smooth gameplay:
- Keep bot AI under 2-3ms average
- No single frame should exceed 5-10ms
*/

// ============================================================
// ADVANCED: Per-Bot Timing
// ============================================================

void BotManager::frameWithTiming() {
    float slowestBot = 0.0f;
    Bot* slowestBotPtr = nullptr;

    for (auto &bot : m_bots) {
        if (!bot || bot->m_isStale) continue;

        float startTime = game.time();
        bot->frame();
        float elapsed = (game.time() - startTime) * 1000.0f;

        if (elapsed > slowestBot) {
            slowestBot = elapsed;
            slowestBotPtr = bot.get();
        }
    }

    if (slowestBot > 1.0f && slowestBotPtr) {
        ctrl.msg("Slowest bot: %s (%.2f ms)",
                 slowestBotPtr->pev->netname.chars(),
                 slowestBot);
    }
}

// ============================================================
// KEY INSIGHTS
// ============================================================

// 1. StartFrame is called by engine, not by us
//    We just provide the callback

// 2. game.time() returns seconds since map load
//    Use for all timing, not system clock

// 3. Frame rate varies - don't assume fixed rate
//    Always use delta time for movement/physics

// 4. Bot AI is the biggest CPU consumer
//    Must be optimized carefully

// 5. Spread heavy work across frames
//    Use canRunHeavyWeight() pattern
