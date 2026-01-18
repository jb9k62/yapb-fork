# Lesson 06: Smart Pointers & RAII

Learn automatic memory management through RAII and smart pointers - C++'s answer to garbage collection.

## Learning Objectives

- Understand stack vs heap allocation
- Learn the RAII pattern
- Use `UniquePtr` for automatic cleanup
- Trace bot creation/destruction in YaPB

## Theory: Stack vs Heap

### TypeScript: All Objects on Heap

```typescript
// All objects live on the heap, GC handles cleanup
const bot = new Bot();          // Allocated on heap
const pos = { x: 0, y: 0 };     // Heap
const arr = [1, 2, 3];          // Heap

// No cleanup needed - garbage collector handles it
```

### C++: You Choose

```cpp
// Stack allocation (automatic cleanup)
void function() {
    Bot bot;                    // On stack
    Vector pos(0, 0, 0);        // On stack
    int arr[3] = {1, 2, 3};     // On stack
}  // All destroyed here automatically

// Heap allocation (manual cleanup required)
void function() {
    Bot* bot = new Bot();       // On heap
    // ... use bot ...
    delete bot;                 // Must cleanup manually!
}
```

### Stack vs Heap Comparison

| Aspect | Stack | Heap |
|--------|-------|------|
| Speed | Fast | Slower |
| Size limit | ~1MB | Limited by RAM |
| Lifetime | Until scope ends | Until `delete` |
| Cleanup | Automatic | Manual (or smart pointers) |

## Theory: The Problem with Manual Memory

### Memory Leak

```cpp
void leaky() {
    Bot* bot = new Bot();
    if (error) {
        return;  // LEAK! bot never deleted
    }
    delete bot;
}
```

### Double Delete

```cpp
void dangerous() {
    Bot* bot = new Bot();
    delete bot;
    delete bot;  // CRASH! Already deleted
}
```

### Use After Free

```cpp
void unsafe() {
    Bot* bot = new Bot();
    delete bot;
    bot->think();  // CRASH! Memory freed
}
```

## Theory: RAII

**RAII** = Resource Acquisition Is Initialization

The idea: tie resource lifetime to object lifetime.

```cpp
// RAII wrapper
class FileHandle {
    FILE* m_file;
public:
    FileHandle(const char* path) {
        m_file = fopen(path, "r");  // Acquire resource
    }
    ~FileHandle() {
        if (m_file) fclose(m_file); // Release resource
    }
};

void function() {
    FileHandle file("data.txt");  // Resource acquired
    // ... use file ...
}  // Destructor called, resource released
```

This is the **most important** C++ pattern. Destructors guarantee cleanup.

## Theory: Smart Pointers

Smart pointers are RAII wrappers for heap memory.

### unique_ptr (Standard Library)

```cpp
#include <memory>

void function() {
    std::unique_ptr<Bot> bot = std::make_unique<Bot>();
    bot->think();  // Use like regular pointer
}  // Automatically deleted here
```

### UniquePtr (crlib - YaPB's version)

```cpp
void function() {
    UniquePtr<Bot> bot = cr::makeUnique<Bot>();
    bot->think();  // Same usage
}  // Automatically deleted
```

### Key Properties

| Property | Description |
|----------|-------------|
| Unique ownership | Only one UniquePtr can own the object |
| Move only | Cannot copy, only move |
| Auto cleanup | Destructor deletes the object |
| Zero overhead | Same as raw pointer at runtime |

### Move Semantics

```cpp
UniquePtr<Bot> createBot() {
    return cr::makeUnique<Bot>();  // Move out
}

void function() {
    UniquePtr<Bot> bot1 = createBot();
    UniquePtr<Bot> bot2 = cr::move(bot1);  // Move ownership
    // bot1 is now nullptr, bot2 owns the Bot
}
```

## Code Study: BotManager's Bot Storage

### File: `inc/manager.h:24,44`

```cpp
class BotManager final : public Singleton <BotManager> {
public:
   using UniqueBot = UniquePtr <Bot>;

private:
   SmallArray <UniqueBot> m_bots {};  // all available bots
```

YaPB stores bots as `UniquePtr<Bot>` in an array. When a bot is removed:

1. Remove `UniquePtr` from array
2. `UniquePtr` destructor runs
3. Bot is automatically deleted

### Creating a Bot

```cpp
BotCreateResult BotManager::create(...) {
    // Create bot with UniquePtr
    auto bot = cr::makeUnique<Bot>(ent, difficulty, personality, team, skin);

    // Move into array
    m_bots.emplace(cr::move(bot));

    return BotCreateResult::Success;
}
```

### Iterating Bots

```cpp
for (auto &bot : bots) {
    // bot is UniqueBot& (reference to UniquePtr<Bot>)
    bot->think();  // Call method through smart pointer
}
```

## Code Study: PathWalk Memory

### File: `inc/yapb.h:140-211`

```cpp
class PathWalk final : public NonCopyable {
private:
   size_t m_cursor {};
   size_t m_length {};
   UniquePtr <int32_t[]> m_path {};  // Array managed by UniquePtr

public:
   void init (size_t length) {
      m_path = cr::makeUnique <int32_t[]> (length);
   }
```

`UniquePtr<int32_t[]>` manages a dynamically-sized array. When PathWalk is destroyed, the array is automatically freed.

## Exercise: Trace Bot Lifecycle

Add logging to understand when bots are created and destroyed.

### Your Task

1. Find the Bot constructor
2. Find the Bot destructor
3. Add logging messages
4. Observe the output when adding/removing bots

### Step-by-Step

#### Step 1: Find the Constructor

In `src/botlib.cpp`, find the Bot constructor:

```cpp
Bot::Bot (edict_t *bot, int difficulty, int personality, int team, int skin) {
    // ... initialization code
}
```

Add at the start:
```cpp
ctrl.msg("Bot CREATED: %s (difficulty: %d)", bot->v.netname.chars(), difficulty);
```

#### Step 2: Find/Add Destructor

In `inc/yapb.h`, the Bot class has:
```cpp
~Bot () = default;
```

Change it to a declaration:
```cpp
~Bot ();
```

In `src/botlib.cpp`, add the definition:
```cpp
Bot::~Bot() {
    ctrl.msg("Bot DESTROYED: %s", pev->netname.chars());
}
```

#### Step 3: Test

```bash
meson compile -C build
./deploy.sh
```

In game:
```
yb add       // See "Bot CREATED: ..."
yb kick      // See "Bot DESTROYED: ..."
```

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: NonCopyable Pattern

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

Inheriting from `NonCopyable` prevents accidental copies:

```cpp
class Bot : public NonCopyable {
    // Cannot do: Bot b2 = b1;  (copy)
    // Can do: Bot* p = &b1;    (pointer)
    // Can do: Bot& r = b1;     (reference)
};
```

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Stack | Automatic lifetime, fast, limited size |
| Heap | Manual lifetime, slower, unlimited |
| RAII | Resource tied to object lifetime |
| UniquePtr | Smart pointer with unique ownership |
| Destructor | `~ClassName()` - called when object dies |
| NonCopyable | Prevents accidental copies |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript - GC handles cleanup
function createBot(): Bot {
    return new Bot();  // GC will clean up
}

// C++ - Smart pointer handles cleanup
UniquePtr<Bot> createBot() {
    return cr::makeUnique<Bot>();  // RAII cleanup
}
```

```typescript
// TypeScript - no concept of ownership
class BotManager {
    bots: Bot[] = [];
    add(bot: Bot) { this.bots.push(bot); }
}

// C++ - explicit ownership transfer
class BotManager {
    SmallArray<UniquePtr<Bot>> m_bots;
    void add(UniquePtr<Bot> bot) {
        m_bots.emplace(cr::move(bot));  // Transfer ownership
    }
};
```

## Common Mistakes

### Copying UniquePtr

```cpp
UniquePtr<Bot> bot1 = cr::makeUnique<Bot>();
UniquePtr<Bot> bot2 = bot1;  // ERROR! Cannot copy

// Correct: move ownership
UniquePtr<Bot> bot2 = cr::move(bot1);
```

### Using After Move

```cpp
UniquePtr<Bot> bot1 = cr::makeUnique<Bot>();
UniquePtr<Bot> bot2 = cr::move(bot1);
bot1->think();  // ERROR! bot1 is nullptr after move
```

### Raw Pointer from UniquePtr

```cpp
UniquePtr<Bot> owner = cr::makeUnique<Bot>();
Bot* raw = owner.get();  // Get raw pointer
// Be careful! raw becomes invalid when owner is destroyed
```

## What's Next

In Lesson 07, you'll learn about C++ classes in depth:

- Header/implementation split
- Constructors and destructors
- The Bot class structure

**[Next: Lesson 07 - Classes & OOP](../07-classes-oop/README.md)**
