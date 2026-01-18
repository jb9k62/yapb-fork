// Solution for Lesson 09: Inheritance & Virtual Functions
//
// This lesson is primarily conceptual. Here are code examples.

// ============================================================
// EXPERIMENT 1: Try to inherit from Bot (will fail)
// ============================================================

// Create a test file: test_inherit.cpp
/*
#include <yapb.h>

// Attempt to subclass Bot
class SuperBot : public Bot {
public:
    void superThink() {
        // Extra intelligence!
    }
};

// Compile error:
// error: cannot derive from 'final' base 'Bot' in derived type 'SuperBot'
*/

// ============================================================
// EXPERIMENT 2: Remove 'final' and try again
// ============================================================

// In inc/yapb.h, change:
//   class Bot final {
// To:
//   class Bot {

// Then try to compile. You'll get errors about:
// - No accessible constructor
// - Private members
// - Bot isn't designed for inheritance

// ============================================================
// WHY YAPB AVOIDS INHERITANCE
// ============================================================

// APPROACH 1: Inheritance-based (NOT used)
/*
class Bot {
public:
    virtual void selectWeapon() = 0;
    virtual void findEnemy() = 0;
    virtual void navigate() = 0;
};

class AggressiveBot : public Bot {
    void selectWeapon() override { /* prefer assault rifles */ }
    void findEnemy() override { /* rush forward */ }
    void navigate() override { /* take shortest path */ }
};

class DefensiveBot : public Bot {
    void selectWeapon() override { /* prefer sniper */ }
    void findEnemy() override { /* wait for enemies */ }
    void navigate() override { /* stay in cover */ }
};

// Problem: Need different class for each personality
// Problem: Can't change personality at runtime
// Problem: Virtual call overhead every frame
*/

// APPROACH 2: Data-driven (USED by YaPB)
class Bot {
    Personality m_personality;  // Rusher, Normal, Careful
    float m_agressionLevel;     // 0.0 to 1.0
    float m_fearLevel;          // 0.0 to 1.0

    void selectWeapon() {
        if (m_personality == Personality::Rusher) {
            // Prefer assault rifles
        } else if (m_personality == Personality::Careful) {
            // Prefer accurate weapons
        }
    }

    void navigate() {
        if (m_agressionLevel > 0.7f) {
            // Take direct route
        } else if (m_fearLevel > 0.7f) {
            // Take safe route
        }
    }
};

// Benefits:
// - Single class, behavior varies by data
// - Personality can change mid-game
// - No virtual function overhead

// ============================================================
// PROPER USE OF INHERITANCE: NonCopyable
// ============================================================

// This is a valid use of inheritance in YaPB:

class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    // Delete copy operations
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

// Bot inherits from NonCopyable to gain the "no copy" behavior
class Bot final : public NonCopyable {
    // Bot cannot be copied
};

// Why this works:
// - NonCopyable is a "mixin" - adds behavior without polymorphism
// - No virtual functions, no runtime cost
// - Just compile-time enforcement

// ============================================================
// WHEN INHERITANCE IS APPROPRIATE
// ============================================================

// Good use: Code reuse mixin (like NonCopyable)
class Printable {
public:
    void print() const {
        ctrl.msg("%s", toString().chars());
    }
protected:
    virtual String toString() const = 0;
};

// Good use: Interface definition (rare in YaPB)
class IPathfinder {
public:
    virtual ~IPathfinder() = default;
    virtual bool findPath(int from, int to) = 0;
    virtual int getNextNode() = 0;
};

// Good use: Plugin system (not in YaPB)
class PluginBase {
public:
    virtual ~PluginBase() = default;
    virtual void onFrame() = 0;
    virtual void onRoundStart() = 0;
};

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
// TypeScript doesn't have 'final' class keyword, but can prevent inheritance:
class Bot {
    constructor() {
        if (this.constructor !== Bot) {
            throw new Error("Bot cannot be subclassed");
        }
    }
}

// Or using a factory pattern:
const createBot = (): Bot => {
    // Only way to create bots
    return new Bot();
};

// TypeScript equivalent of NonCopyable doesn't exist
// Because TypeScript objects are always references
// No accidental copying occurs
*/

// ============================================================
// KEY TAKEAWAYS
// ============================================================

// 1. Use 'final' when you don't want inheritance
//    - Prevents accidental subclassing
//    - Allows compiler optimization
//    - Documents design intent

// 2. Prefer composition over inheritance
//    - Change behavior with data, not subclasses
//    - More flexible at runtime
//    - Simpler to understand

// 3. Use inheritance for:
//    - Mixins (NonCopyable, Printable)
//    - True is-a relationships
//    - Plugin/extension systems

// 4. YaPB's design philosophy:
//    - Bot is a complete implementation, not a base class
//    - Personality is data, not type
//    - Singletons (Game, BotManager) are complete systems
