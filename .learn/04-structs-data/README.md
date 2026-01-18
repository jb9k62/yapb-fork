# Lesson 04: Structs & Data Organization

Learn how C++ organizes data through structs by examining YaPB's task and weapon systems.

## Learning Objectives

- Understand structs vs TypeScript interfaces
- Learn about memory layout and initialization
- Explore the `BotTask` and `WeaponInfo` structures
- Create a custom struct to track bot statistics

## Theory: Structs vs Interfaces

### TypeScript Interface

```typescript
interface BotTask {
    id: TaskId;
    desire: number;
    data: number;
    time: number;
    resume: boolean;
}

// Usage
const task: BotTask = {
    id: TaskId.Normal,
    desire: 35.0,
    data: 0,
    time: 0,
    resume: false
};
```

TypeScript interfaces only exist at compile time - they're erased at runtime.

### C++ Struct

```cpp
struct BotTask {
    Task id {};
    float desire {};
    int data {};
    float time {};
    bool resume {};
};

// Usage
BotTask task {
    Task::Normal,
    35.0f,
    0,
    0.0f,
    false
};
```

C++ structs are actual memory layouts - they define exactly how data is stored.

### Key Differences

| Aspect | TypeScript Interface | C++ Struct |
|--------|----------------------|------------|
| Runtime | Erased | Real memory |
| Memory layout | N/A | Defined |
| Methods | Not possible | Allowed |
| Defaults | Not possible | Possible with `{}` |
| Inheritance | Extends interface | Can inherit |

## Theory: Memory Layout

In C++, structs have a specific memory layout:

```cpp
struct Example {
    int32_t a;    // 4 bytes at offset 0
    float b;      // 4 bytes at offset 4
    bool c;       // 1 byte at offset 8
    // 3 bytes padding for alignment
    int32_t d;    // 4 bytes at offset 12
};
// Total size: 16 bytes
```

### Alignment

CPUs read memory most efficiently when data is aligned:
- `int32_t` should be at addresses divisible by 4
- `float` should be at addresses divisible by 4
- The compiler adds padding to ensure alignment

### Why This Matters

```cpp
// Inefficient layout (20 bytes with padding)
struct Bad {
    bool a;      // 1 byte + 3 padding
    int32_t b;   // 4 bytes
    bool c;      // 1 byte + 3 padding
    int32_t d;   // 4 bytes
    bool e;      // 1 byte + 3 padding
};

// Efficient layout (12 bytes, no padding)
struct Good {
    int32_t b;   // 4 bytes
    int32_t d;   // 4 bytes
    bool a;      // 1 byte
    bool c;      // 1 byte
    bool e;      // 1 byte + 1 padding
};
```

YaPB carefully orders struct members for efficiency.

## Theory: Initialization

### Default Member Initialization

```cpp
struct Client {
    edict_t *ent {};       // nullptr
    Vector origin {};      // (0, 0, 0)
    int team {};           // 0
    int flags {};          // 0
};
```

The `{}` provides default initialization:
- Pointers become `nullptr`
- Numbers become `0`
- Objects use their default constructor

### Brace Initialization

```cpp
// Named members (C++20 designated initializers)
BotTask task {
    .id = Task::Normal,
    .desire = 35.0f,
    .data = nodeIndex,
    .time = 0.0f,
    .resume = true
};

// Positional (older style)
BotTask task { Task::Normal, 35.0f, nodeIndex, 0.0f, true };
```

## Code Study: BotTask Structure

### File: `inc/yapb.h:25-38`

```cpp
// tasks definition
struct BotTask {
   using Function = void (Bot:: *) ();

public:
   Function func {}; // corresponding exec function in bot class
   Task id {};       // major task/action carried out
   float desire {};  // desire (filled in) for this task
   int data {};      // additional data (node index)
   float time {};    // time task expires
   bool resume {};   // if task can be continued if interrupted

public:
   BotTask (Function func, Task id, float desire, int data, float time, bool resume)
       : func (func), id (id), desire (desire), data (data), time (time), resume (resume) {}
};
```

### Breaking It Down

#### Type Alias
```cpp
using Function = void (Bot:: *) ();
```
This creates a type alias for a pointer to a Bot member function. Think of it as:
```typescript
type Function = (this: Bot) => void;
```

#### Constructor with Initializer List
```cpp
BotTask (Function func, Task id, float desire, int data, float time, bool resume)
    : func (func), id (id), desire (desire), data (data), time (time), resume (resume) {}
```

The `: member(value), ...` syntax is an **initializer list**. It initializes members before the constructor body runs.

### TypeScript Equivalent

```typescript
interface BotTask {
    func: () => void;
    id: Task;
    desire: number;
    data: number;
    time: number;
    resume: boolean;
}

class BotTaskImpl implements BotTask {
    constructor(
        public func: () => void,
        public id: Task,
        public desire: number,
        public data: number,
        public time: number,
        public resume: boolean
    ) {}
}
```

## Code Study: WeaponInfo Structure

### File: `inc/yapb.h:52-88`

```cpp
struct WeaponInfo {
   int id {};
   StringRef name {};
   StringRef model {};
   StringRef alias {};
   int price {};
   int minPrimaryAmmo {};
   int teamStandard {};
   int teamAS {};
   int buyGroup {};
   int buySelect {};
   int buySelectT {};
   int buySelectCT {};
   int penetratePower {};
   int maxClip {};
   int type {};
   bool primaryFireHold {};

public:
   WeaponInfo (int id, StringRef name, StringRef model, int price,
               int minPriAmmo, int teamStd, int teamAs, int buyGroup,
               int buySelect, int buySelectT, int buySelectCT,
               int penetratePower, int maxClip, int type, bool fireHold)
       : id (id), name (name), model (model), price (price),
         // ... initialization continues
         {}
};
```

This structure holds all data about a weapon - used for weapon selection and buying logic.

## Code Study: Client Structure

### File: `inc/yapb.h:98-109`

```cpp
struct Client {
   edict_t *ent {};                   // pointer to actual edict
   Vector origin {};                  // position in the world
   int team {};                       // bot team
   int team2 {};                      // real team in FFA mode
   int flags {};                      // client flags
   int radio {};                      // radio orders
   int menu {};                       // identifier to opened menu
   int iconFlags[kGameMaxPlayers] {}; // flag holding chatter icons
   float iconTimestamp[kGameMaxPlayers] {}; // timers
   ClientNoise noise {};
};
```

This tracks information about each player in the game.

## Exercise: Create Bot Stats Struct

Create a structure to track bot performance statistics.

### Your Task

1. Define a `BotStats` struct with kill/death tracking
2. Add it to the Bot class
3. Update stats when events occur
4. Print stats periodically

### Step-by-Step

#### Step 1: Define the Struct

In `inc/yapb.h`, before the Bot class definition (around line 140), add:

```cpp
struct BotStats {
    int kills {};           // number of kills
    int deaths {};          // number of deaths
    int headshots {};       // headshot kills
    float damageDealt {};   // total damage dealt
    float damageTaken {};   // total damage taken

    float getKDRatio() const {
        if (deaths == 0) return static_cast<float>(kills);
        return static_cast<float>(kills) / static_cast<float>(deaths);
    }

    void reset() {
        kills = 0;
        deaths = 0;
        headshots = 0;
        damageDealt = 0.0f;
        damageTaken = 0.0f;
    }
};
```

#### Step 2: Add to Bot Class

In the Bot class public members (around line 720), add:

```cpp
BotStats m_stats {};  // bot performance statistics
```

#### Step 3: Print Stats Function

Add a method to print stats. Declaration in `inc/yapb.h`:
```cpp
void printStats();
```

Definition in `src/combat.cpp`:
```cpp
void Bot::printStats() {
    ctrl.msg("%s - K/D: %d/%d (%.2f), HS: %d",
             pev->netname.chars(),
             m_stats.kills,
             m_stats.deaths,
             m_stats.getKDRatio(),
             m_stats.headshots);
}
```

### Build and Test

The stats won't automatically update (that requires hooking into kill events), but you've created the data structure.

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: Struct vs Class

In C++, `struct` and `class` are almost identical:

```cpp
struct Foo {
    int x;          // public by default
};

class Bar {
    int x;          // private by default
public:
    int getX() { return x; }
};
```

**Convention:**
- Use `struct` for plain data containers (POD - Plain Old Data)
- Use `class` for objects with complex behavior

YaPB follows this convention - simple data uses `struct`, complex AI uses `class`.

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Struct | Fixed memory layout data structure |
| Memory alignment | Data aligned for CPU efficiency |
| `{}` initialization | Default value initialization |
| Initializer list | `: member(value)` syntax |
| `using` alias | Type alias for complex types |
| Struct methods | Functions inside structs |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript interface
interface Point {
    x: number;
    y: number;
}

// C++ struct
struct Point {
    float x {};
    float y {};
};
```

```typescript
// TypeScript with method
interface Stats {
    kills: number;
    deaths: number;
    getRatio(): number;
}

// C++ struct with method
struct Stats {
    int kills {};
    int deaths {};

    float getRatio() const {
        return deaths > 0 ? float(kills) / deaths : float(kills);
    }
};
```

## Common Mistakes

### Forgetting `{}`

```cpp
struct Bad {
    int x;        // Uninitialized! Contains garbage
    int y;
};

struct Good {
    int x {};     // Initialized to 0
    int y {};
};
```

### Wrong Initialization Order

```cpp
struct Foo {
    int a;
    int b;
    Foo() : b(1), a(b) {}  // Warning! a initialized before b
};
```

Initializer list runs in declaration order, not the order written.

## What's Next

In Lesson 05, you'll learn about pointers and references:

- Memory addresses and dereferencing
- `nullptr` and null safety
- `->` vs `.` operators
- How YaPB handles entity pointers

**[Next: Lesson 05 - Pointers & References](../05-pointers-references/README.md)**
