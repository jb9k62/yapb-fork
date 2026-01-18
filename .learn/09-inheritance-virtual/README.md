# Lesson 09: Inheritance & Virtual Functions

Understand why YaPB deliberately avoids inheritance and uses `final` on most classes.

## Learning Objectives

- Understand C++ inheritance syntax
- Learn about virtual functions and polymorphism
- Understand why YaPB uses `final` everywhere
- Learn from the `NonCopyable` pattern
- Know when inheritance is appropriate

## Theory: Inheritance Basics

### TypeScript Inheritance

```typescript
class Entity {
    protected position: Vector;

    move(dir: Vector): void {
        this.position = this.position.add(dir);
    }
}

class Bot extends Entity {
    private health: number;

    attack(): void {
        // Bots can attack
    }
}
```

### C++ Inheritance

```cpp
class Entity {
protected:
    Vector m_position;

public:
    void move(const Vector& dir) {
        m_position += dir;
    }
};

class Bot : public Entity {  // public inheritance
private:
    float m_health;

public:
    void attack() {
        // Bot-specific behavior
    }
};
```

### Inheritance Types

| Type | Syntax | Base Public | Base Protected |
|------|--------|-------------|----------------|
| Public | `: public Base` | Public | Protected |
| Protected | `: protected Base` | Protected | Protected |
| Private | `: private Base` | Private | Private |

Almost always use `public` inheritance.

## Theory: Virtual Functions

### The Problem

```cpp
class Entity {
public:
    void think() {
        // Default behavior
    }
};

class Bot : public Entity {
public:
    void think() {
        // Bot-specific behavior
    }
};

// Problem:
Entity* e = new Bot();
e->think();  // Calls Entity::think(), not Bot::think()!
```

### The Solution: virtual

```cpp
class Entity {
public:
    virtual void think() {
        // Default behavior
    }
};

class Bot : public Entity {
public:
    void think() override {  // override keyword for safety
        // Bot-specific behavior
    }
};

Entity* e = new Bot();
e->think();  // Now calls Bot::think()
```

### virtual Mechanics

```
┌─────────────────────────────────────┐
│ Non-virtual call                    │
│                                     │
│ Entity* e = new Bot();              │
│ e->think();                         │
│      │                              │
│      └──> Entity::think() (direct)  │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│ Virtual call                        │
│                                     │
│ Entity* e = new Bot();              │
│ e->think();                         │
│      │                              │
│      └──> vtable lookup             │
│              └──> Bot::think()      │
└─────────────────────────────────────┘
```

Virtual calls have a small performance cost (vtable lookup).

## Theory: The `final` Keyword

### Preventing Inheritance

```cpp
class Bot final : public Entity {
    // Cannot be inherited from
};

class SuperBot : public Bot {  // ERROR!
};
```

### Preventing Override

```cpp
class Bot : public Entity {
public:
    void think() final {  // Cannot be overridden
        // ...
    }
};
```

## Code Study: Why YaPB Uses `final`

### File: `inc/yapb.h:214`

```cpp
class Bot final {  // Note: final
    // ...
};
```

### File: `inc/manager.h:21`

```cpp
class BotManager final : public Singleton<BotManager> {
    // ...
};
```

### File: `inc/graph.h:148`

```cpp
class BotGraph final : public Singleton<BotGraph> {
    // ...
};
```

### Why `final` Everywhere?

1. **Performance**: Compiler can devirtualize calls
2. **Simplicity**: No need to think about subclass behavior
3. **Design**: These classes represent complete implementations
4. **Bugs**: Prevents accidental inheritance problems

### When YaPB Would NOT Use `final`

If YaPB supported different AI implementations:

```cpp
// Hypothetical - not in actual YaPB
class BotAI {
public:
    virtual void think() = 0;          // Pure virtual
    virtual void selectWeapon() = 0;
};

class AggressiveAI : public BotAI {
public:
    void think() override { /* Rush! */ }
    void selectWeapon() override { /* Big guns */ }
};

class CautiousAI : public BotAI {
public:
    void think() override { /* Camp */ }
    void selectWeapon() override { /* Sniper */ }
};
```

But YaPB doesn't do this - personality is a data parameter, not a class hierarchy.

## Code Study: NonCopyable Pattern

### File: `ext/crlib/crlib/basic.h`

```cpp
class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
```

This is a utility base class that:
- Prevents copy construction
- Prevents copy assignment
- Has no virtual functions (not for polymorphism)

### Using NonCopyable

```cpp
class Bot final : public NonCopyable {
    // Bot cannot be copied
};

Bot bot1;
Bot bot2 = bot1;  // ERROR! Copy constructor deleted
```

### Why Prevent Copying?

```cpp
// Dangerous if allowed:
Bot original;
original.m_enemy = someEnemy;
original.m_planner = makeUnique<AStarAlgo>();

Bot copy = original;  // What happens?
// - m_enemy: shared (both point to same enemy)
// - m_planner: UniquePtr can't be copied!

// Better to prevent and force explicit cloning if needed
```

## Exercise: Understand final Behavior

Try to subclass Bot and see the compiler error.

### Your Task

1. Create a test file that tries to inherit from Bot
2. Observe the compiler error
3. Remove `final` temporarily and see what happens

### Test Code

Create `test_inherit.cpp`:

```cpp
#include <yapb.h>

// Try to inherit from Bot
class SuperBot : public Bot {  // This should fail
public:
    void superThink() {
        // Extra smart!
    }
};
```

### Expected Error

```
error: cannot derive from 'final' base 'Bot' in derived type 'SuperBot'
```

### Experiment: Remove `final`

1. In `inc/yapb.h`, change `class Bot final` to `class Bot`
2. Try to compile your test
3. Observe what errors you get (likely about constructors)

This shows that even without `final`, Bot isn't designed for inheritance - the constructor is not virtual, members are private, etc.

## Solution

See `solution.cpp` for discussion and examples.

## Deep Dive: Composition Over Inheritance

YaPB prefers **composition**:

```cpp
// Instead of:
class AgressiveBot : public Bot { };
class CautiousBot : public Bot { };

// YaPB does:
class Bot {
    Personality m_personality;  // Rusher, Normal, Careful
    int m_difficulty;           // 0-4

    void think() {
        // Behavior varies based on personality/difficulty
        if (m_personality == Personality::Rusher) {
            rushToEnemy();
        }
    }
};
```

Benefits:
- Can change personality at runtime
- Simpler code structure
- No virtual function overhead

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| `: public Base` | Public inheritance |
| `virtual` | Enable runtime polymorphism |
| `override` | Explicitly override base method |
| `final` (class) | Prevent inheritance |
| `final` (method) | Prevent override |
| `= delete` | Delete a function |
| NonCopyable | Pattern to prevent copying |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript
class Entity {
    move(): void { }
}
class Bot extends Entity {
    think(): void { }
}

// C++
class Entity {
public:
    virtual void move() {}
};
class Bot : public Entity {
public:
    void think() {}
};
```

```typescript
// TypeScript (no final concept)
class Bot { }

// C++
class Bot final { };  // Cannot inherit
```

## What's Next

In Lesson 10, you'll learn about the game loop:

- Frame-based execution
- The `StartFrame` callback
- How bots think every frame

**[Next: Lesson 10 - The Game Loop](../10-game-loop/README.md)**
