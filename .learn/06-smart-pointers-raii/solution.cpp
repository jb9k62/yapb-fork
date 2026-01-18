// Solution for Lesson 06: Smart Pointers & RAII
//
// This exercise traces bot creation and destruction.

// ============================================================
// STEP 1: Modify constructor in src/botlib.cpp
// ============================================================
// Find the Bot constructor and add logging at the start:

Bot::Bot (edict_t *bot, int difficulty, int personality, int team, int skin)
    : /* existing initializer list */ {

    // Add this log at the start of constructor body:
    ctrl.msg("Bot CREATED: %s (diff: %d, team: %d)",
             bot->v.netname.chars(),
             difficulty,
             team);

    // ... rest of existing constructor code ...
}

// ============================================================
// STEP 2: Change destructor declaration in inc/yapb.h
// ============================================================
// Find this line in the Bot class:
//    ~Bot () = default;
// Change it to:
//    ~Bot ();

// ============================================================
// STEP 3: Add destructor definition in src/botlib.cpp
// ============================================================

Bot::~Bot() {
    // Note: pev might be invalid if entity was already freed
    // Use a safer approach:
    if (pev && pev->pContainingEntity) {
        ctrl.msg("Bot DESTROYED: %s (played %.0f seconds)",
                 pev->netname.chars(),
                 m_playServerTime);
    } else {
        ctrl.msg("Bot DESTROYED: (entity already freed)");
    }
}

// ============================================================
// WHAT YOU'LL SEE IN GAME
// ============================================================

/*
Console output when adding bots:

> yb add
Bot CREATED: Alfred (diff: 2, team: 1)

> yb add
Bot CREATED: Boris (diff: 3, team: 2)

> yb kick
Bot DESTROYED: Alfred (played 45 seconds)

> yb kick all
Bot DESTROYED: Boris (played 60 seconds)

On map change or server shutdown:
Bot DESTROYED: Alfred (played 120 seconds)
Bot DESTROYED: Boris (played 120 seconds)
*/

// ============================================================
// UNDERSTANDING THE LIFECYCLE
// ============================================================

// 1. Bot is created in BotManager::create():
//    auto bot = cr::makeUnique<Bot>(ent, difficulty, ...);
//    m_bots.emplace(cr::move(bot));
//
//    - UniquePtr allocated
//    - Bot constructor runs
//    - Ownership transferred to m_bots array

// 2. Bot lives in the array, frame() called each tick:
//    for (auto &bot : m_bots) {
//        bot->frame();
//    }

// 3. Bot is removed in BotManager::disconnectBot():
//    m_bots.erase(...);
//
//    - UniquePtr removed from array
//    - UniquePtr destructor runs
//    - Bot destructor runs
//    - Memory freed

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class Bot {
    private name: string;
    private playTime: number = 0;

    constructor(entity: Entity, difficulty: number, team: number) {
        this.name = entity.name;
        console.log(`Bot CREATED: ${this.name} (diff: ${difficulty})`);

        // In TypeScript, no destructor - would use a dispose pattern:
        // But GC handles memory automatically
    }

    // TypeScript equivalent of destructor - manual dispose pattern
    dispose(): void {
        console.log(`Bot DESTROYED: ${this.name}`);
        // Cleanup resources...
    }
}

class BotManager {
    private bots: Bot[] = [];

    kick(bot: Bot): void {
        bot.dispose();  // Must call manually!
        this.bots = this.bots.filter(b => b !== bot);
        // In C++, UniquePtr calls destructor automatically
    }
}
*/

// ============================================================
// KEY RAII CONCEPTS DEMONSTRATED
// ============================================================

// 1. Constructor = resource acquisition
//    Bot::Bot() acquires:
//    - Entity slot in game
//    - Memory for bot state
//    - A* pathfinder instance

// 2. Destructor = resource release
//    Bot::~Bot() should release:
//    - Any held resources
//    - Log final stats
//    - Cleanup state

// 3. UniquePtr guarantees destructor is called
//    When UniquePtr goes out of scope or is removed from array,
//    the destructor ALWAYS runs (even during exceptions).

// 4. No manual delete needed
//    BotManager never calls: delete bot;
//    The UniquePtr handles it automatically.

// ============================================================
// ADDITIONAL TRACING (OPTIONAL)
// ============================================================

// You can also trace the UniquePtr itself:

template<typename T>
class TrackedUniquePtr {
    UniquePtr<T> m_ptr;
public:
    template<typename... Args>
    TrackedUniquePtr(Args&&... args)
        : m_ptr(cr::makeUnique<T>(cr::forward<Args>(args)...)) {
        ctrl.msg("UniquePtr ACQUIRED");
    }

    ~TrackedUniquePtr() {
        ctrl.msg("UniquePtr RELEASED");
    }

    T* operator->() { return m_ptr.get(); }
    T& operator*() { return *m_ptr; }
};

// This shows when ownership is transferred vs when objects are destroyed.
