# Lesson 07: Classes & OOP

Explore object-oriented programming in C++ through the Bot class - YaPB's central component.

## Learning Objectives

- Understand header/implementation file separation
- Learn about constructors and destructors
- Understand access specifiers (public/private/protected)
- Explore the Bot class architecture
- Add a new method to the Bot class

## Theory: Classes vs TypeScript Classes

### TypeScript Class

```typescript
class Bot {
    private health: number;
    public name: string;

    constructor(name: string) {
        this.name = name;
        this.health = 100;
    }

    takeDamage(amount: number): void {
        this.health -= amount;
    }

    getHealth(): number {
        return this.health;
    }
}
```

### C++ Class

```cpp
class Bot {
private:
    float m_health;

public:
    String m_name;

    Bot(const char* name);
    ~Bot();

    void takeDamage(float amount);
    float getHealth() const;
};
```

## Theory: Header vs Implementation

C++ separates **declaration** (what exists) from **definition** (how it works).

### Header File (`.h`)

```cpp
// bot.h - Declaration
class Bot {
private:
    float m_health;
    String m_name;

public:
    Bot(const char* name);      // Constructor declaration
    void takeDamage(float amt); // Method declaration
    float getHealth() const;    // Method declaration
};
```

### Implementation File (`.cpp`)

```cpp
// bot.cpp - Definition
#include "bot.h"

Bot::Bot(const char* name) : m_name(name), m_health(100.0f) {
    // Constructor body
}

void Bot::takeDamage(float amt) {
    m_health -= amt;
}

float Bot::getHealth() const {
    return m_health;
}
```

### Why Separate?

| Reason | Explanation |
|--------|-------------|
| Compilation speed | Only recompile `.cpp` files that changed |
| Encapsulation | Users see interface, not implementation |
| Organization | Large classes become manageable |
| Circular dependencies | Headers can forward-declare |

## Theory: Access Specifiers

```cpp
class Bot {
public:      // Anyone can access
    void think();

protected:   // Class and subclasses can access
    void plan();

private:     // Only this class can access
    float m_health;
};
```

### TypeScript Comparison

| TypeScript | C++ | Access |
|------------|-----|--------|
| `public` | `public:` | Everyone |
| `protected` | `protected:` | Class + subclasses |
| `private` | `private:` | Class only |
| `#field` | `private:` | True private |

## Theory: Constructors and Initializer Lists

### TypeScript Constructor

```typescript
class Bot {
    constructor(name: string, health: number = 100) {
        this.name = name;
        this.health = health;
    }
}
```

### C++ Constructor with Initializer List

```cpp
Bot::Bot(const char* name, float health)
    : m_name(name),      // Initialize m_name
      m_health(health),  // Initialize m_health
      m_isAlive(true)    // Initialize m_isAlive
{
    // Constructor body runs AFTER initialization
    setup();
}
```

The `: member(value), ...` syntax is the **initializer list**. It's more efficient than assignment in the body.

### Why Initializer Lists?

```cpp
// Bad: Default construct, then assign
Bot::Bot(const char* name) {
    m_name = name;  // Default construct String, then copy
}

// Good: Direct initialization
Bot::Bot(const char* name) : m_name(name) {
    // m_name constructed directly with value
}
```

## Code Study: The Bot Class

### File: `inc/yapb.h:214-909`

The Bot class is ~700 lines. Let's examine its structure:

```cpp
class Bot final {
public:
   friend class BotManager;  // BotManager can access private members

private:
   // Synchronization
   mutable Mutex m_pathFindLock {};
   mutable Mutex m_predictLock {};

   // State
   uint32_t m_states {};
   uint32_t m_aimFlags {};
   // ... many more private members

private:
   // Private methods - internal logic
   int findNearestNode ();
   bool lookupEnemies ();
   void update ();
   // ...

public:
   // Public state - accessible by other systems
   entvars_t *pev {};
   int m_index {};
   int m_difficulty {};
   // ...

   // Public methods
   Bot (edict_t *bot, int difficulty, int personality, int team, int skin);
   ~Bot () = default;

   void logic ();
   void spawned ();
   void newRound ();
   // ...
};
```

### Key Observations

1. **`final` keyword**: Bot cannot be subclassed
2. **`friend class`**: BotManager has special access
3. **`mutable`**: Can be modified even in const methods
4. **Mixed access**: Some members public for external use

### Member Naming Conventions

```cpp
int m_index;        // m_ prefix for member variables
float m_health;     // Distinguishes from parameters/locals
bool m_isAlive;     // Clear ownership
```

## Code Study: Constructor

### File: `src/botlib.cpp` (Bot constructor)

```cpp
Bot::Bot (edict_t *bot, int difficulty, int personality, int team, int skin) {
    // Store entity pointer
    pev = &bot->v;
    m_index = game.indexOfEntity (bot) - 1;

    // Initialize state
    m_difficulty = difficulty;
    m_personality = static_cast <Personality> (personality);
    m_team = team;

    // Create pathfinder
    m_planner = cr::makeUnique <AStarAlgo> (this);

    // Initialize path storage
    m_pathWalk.init (kMaxNodes);

    // ... more initialization
}
```

## Exercise: Add a reportStatus Method

Create a method that reports the bot's current status.

### Your Task

1. Declare `reportStatus()` in the Bot class header
2. Implement it in the source file
3. Call it from somewhere to test

### Step-by-Step

#### Step 1: Add Declaration

In `inc/yapb.h`, in the Bot class public methods section:

```cpp
void reportStatus() const;
```

#### Step 2: Add Implementation

In `src/botlib.cpp`:

```cpp
void Bot::reportStatus() const {
    const char* taskName = "Unknown";

    // Get current task name
    switch (getCurrentTaskId()) {
        case Task::Normal:    taskName = "Normal";    break;
        case Task::Attack:    taskName = "Attack";    break;
        case Task::Camp:      taskName = "Camp";      break;
        case Task::Hunt:      taskName = "Hunt";      break;
        case Task::SeekCover: taskName = "SeekCover"; break;
        default:              taskName = "Other";     break;
    }

    ctrl.msg("[%s] HP: %.0f | Task: %s | Node: %d | Enemies: %d",
             pev->netname.chars(),
             pev->health,
             taskName,
             m_currentNodeIndex,
             m_numEnemiesLeft);
}
```

#### Step 3: Test It

Add a temporary call in `Bot::logic()`:

```cpp
void Bot::logic() {
    // Add temporarily for testing
    static float nextReport = 0.0f;
    if (game.time() > nextReport) {
        reportStatus();
        nextReport = game.time() + 5.0f;  // Every 5 seconds
    }

    // ... rest of existing code
}
```

### Build and Test

```bash
meson compile -C build
./deploy.sh
```

Watch the console for status reports every 5 seconds.

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: The `final` Keyword

```cpp
class Bot final {  // Cannot be inherited
    // ...
};

// Error: Cannot inherit from final class
class SuperBot : public Bot { };
```

YaPB uses `final` because:
1. Bot is complex - inheritance would be error-prone
2. No need for polymorphism
3. Compiler can optimize better

## Deep Dive: The `friend` Declaration

```cpp
class Bot final {
public:
    friend class BotManager;
private:
    int m_secret;  // BotManager can access this
};
```

`friend` grants full access to another class. Use sparingly - it breaks encapsulation.

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Header/Impl split | Declaration in `.h`, definition in `.cpp` |
| Access specifiers | `public`, `private`, `protected` |
| Initializer list | `: member(value)` efficient initialization |
| `final` | Prevents inheritance |
| `friend` | Grants access to private members |
| `const` method | Method doesn't modify object |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript
class Bot {
    private health: number = 100;

    constructor(private name: string) {}

    getHealth(): number {
        return this.health;
    }
}

// C++
class Bot {
private:
    float m_health = 100.0f;
    String m_name;

public:
    Bot(const char* name) : m_name(name) {}

    float getHealth() const {
        return m_health;
    }
};
```

## Common Mistakes

### Forgetting `const` for Getters

```cpp
// Bad - allows modification in getter
float getHealth() { return m_health; }

// Good - const documents intent
float getHealth() const { return m_health; }
```

### Assignment Instead of Initialization

```cpp
// Inefficient
Bot::Bot(const char* name) {
    m_name = name;  // Default construct, then copy
}

// Efficient
Bot::Bot(const char* name) : m_name(name) {
    // Direct construction
}
```

## What's Next

In Lesson 08, you'll learn about the Singleton pattern:

- Single instance management
- Global access to subsystems
- `CR_EXPOSE_GLOBAL_SINGLETON` macro

**[Next: Lesson 08 - Singleton Pattern](../08-singleton-pattern/README.md)**
