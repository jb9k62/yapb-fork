# Lesson 03: Functions & Control Flow

Learn function syntax and control flow by exploring YaPB's combat decision logic.

## Learning Objectives

- Understand C++ function syntax vs TypeScript
- Learn about return types and void
- Master if/else and switch statements
- Add a debug function that prints enemy information

## Theory: Function Syntax

### TypeScript Functions

```typescript
// Function declaration
function add(a: number, b: number): number {
    return a + b;
}

// Arrow function
const add = (a: number, b: number): number => a + b;

// Optional/default parameters
function greet(name: string = "World"): void {
    console.log(`Hello, ${name}`);
}
```

### C++ Functions

```cpp
// Function declaration (prototype)
int add(int a, int b);

// Function definition
int add(int a, int b) {
    return a + b;
}

// Default parameters
void greet(const char* name = "World") {
    engfuncs.pfnServerPrint(name);
}
```

### Key Differences

| Aspect | TypeScript | C++ |
|--------|------------|-----|
| Return type | After `:` | Before name |
| Void | Optional | Required for no return |
| Overloading | Not supported | Supported |
| Declaration | Optional | Often required (headers) |

### Function Declarations vs Definitions

```cpp
// Declaration (prototype) - in header file (.h)
int numEnemiesNear(const Vector &origin, float radius) const;

// Definition (implementation) - in source file (.cpp)
int Bot::numEnemiesNear(const Vector &origin, float radius) const {
    int count = 0;
    // ... implementation
    return count;
}
```

The declaration tells the compiler "this function exists with this signature."
The definition provides the actual code.

## Theory: Control Flow

### If/Else

TypeScript and C++ are nearly identical:

```typescript
// TypeScript
if (health < 20) {
    retreat();
} else if (health < 50) {
    beCareful();
} else {
    attack();
}
```

```cpp
// C++
if (health < 20) {
    retreat();
} else if (health < 50) {
    beCareful();
} else {
    attack();
}
```

### Switch Statements

```cpp
// TypeScript
switch (weapon) {
    case Weapon.Rifle:
        return "rifle";
    case Weapon.Pistol:
        return "pistol";
    default:
        return "unknown";
}
```

```cpp
// C++
switch (weapon) {
    case Weapon::Rifle:
        return "rifle";
    case Weapon::Pistol:
        return "pistol";
    default:
        return "unknown";
}
```

Note: C++ uses `::` for enum values (scope resolution), not `.`

### Ternary Operator

Both languages support it identically:

```cpp
int damage = isCritical ? baseDamage * 2 : baseDamage;
```

## Code Study: Friend/Enemy Counting

### File: `src/combat.cpp:22-60`

```cpp
int Bot::numFriendsNear(const Vector &origin, const float radius) const {
   if (game.is(GameFlags::FreeForAll)) {
      return 0; // no friends on free for all mode
   }

   int count = 0;
   const float radiusSq = cr::sqrf(radius);

   for (const auto &client : util.getClients()) {
      if (!(client.flags & ClientFlags::Used)
          || !(client.flags & ClientFlags::Alive)
          || client.team != m_team
          || client.ent == ent()) {
         continue;
      }

      if (client.origin.distanceSq(origin) < radiusSq) {
         count++;
      }
   }
   return count;
}
```

### Breaking It Down

```cpp
int Bot::numFriendsNear(const Vector &origin, const float radius) const
│   │                   │                      │                   │
│   │                   │                      │                   └── const method
│   │                   │                      └── radius parameter
│   │                   └── origin parameter (const reference)
│   └── Class name (this is a method of Bot)
└── Return type
```

#### const Reference Parameters

```cpp
const Vector &origin   // Pass by reference, cannot modify
const float radius     // Pass by value (small type, okay to copy)
```

- `const Vector &origin` - Avoids copying the Vector, but can't modify it
- `const float radius` - Copy the float (small, efficient)

#### const Method

```cpp
int numFriendsNear(...) const
                        ^^^^^
```

The `const` after the parameter list means this method doesn't modify the object. It only reads data.

#### Range-Based For Loop

```cpp
for (const auto &client : util.getClients())
```

This is like TypeScript's `for (const client of getClients())`.

- `auto` - Compiler infers the type
- `&` - Reference (no copy)
- `const` - Won't modify client

#### Bitwise Flag Checking

```cpp
if (!(client.flags & ClientFlags::Used))
```

This checks if a bit is set:
- `client.flags & ClientFlags::Used` - Bitwise AND
- `!(...)` - Not (if bit is NOT set)

In TypeScript:
```typescript
if (!(client.flags & ClientFlags.Used))
```

## Code Study: Bot Task Functions

### File: `src/tasks.cpp:18-100` (normal_ function, excerpt)

```cpp
void Bot::normal_() {
    m_aimFlags |= AimFlags::Nav;

    if (m_reloadState == Reload::None && getAmmoInClip() == 0) {
        m_reloadState = Reload::Primary;
    }

    // Check if we have a goal
    if (hasActiveGoal()) {
        // ... goal-related logic
    }

    // Movement logic
    if (m_moveToGoal) {
        // ... pathfinding
    }
}
```

This is the bot's "normal" task - what it does when not in combat or performing special actions.

## Exercise: Add Enemy Count Debug Function

Create a function that prints how many enemies are near the bot.

### Your Task

1. Add a new method `debugEnemyCount()` to the Bot class
2. Call it from a suitable location (like `normal_()`)
3. Print the count to server console

### Step-by-Step

#### Step 1: Add Declaration

In `inc/yapb.h`, find the public methods section of the Bot class (around line 760) and add:

```cpp
void debugEnemyCount();
```

#### Step 2: Add Definition

In `src/combat.cpp`, add the implementation:

```cpp
void Bot::debugEnemyCount() {
    int enemies = numEnemiesNear(pev->origin, 1000.0f);

    if (enemies > 0) {
        ctrl.msg("Bot %s sees %d enemies nearby!",
                 pev->netname.chars(), enemies);
    }
}
```

#### Step 3: Call the Function

In `src/tasks.cpp`, inside `normal_()`, add:

```cpp
void Bot::normal_() {
    // Add at the start of the function
    debugEnemyCount();

    // ... rest of existing code
}
```

### Build and Test

```bash
meson compile -C build
./deploy.sh
```

Start a game with bots on both teams. You should see messages like:
```
Bot Alfred sees 2 enemies nearby!
```

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: Method Qualifiers

### const Methods

```cpp
int getHealth() const {     // Doesn't modify object
    return m_health;
}

void takeDamage(int dmg) {  // Modifies object
    m_health -= dmg;
}
```

### static Methods

```cpp
class Bot {
    static int totalBots;  // Shared across all instances

    static int getTotalBots() {  // No 'this' pointer
        return totalBots;
    }
};
```

### Virtual Methods (preview)

```cpp
virtual void think() {      // Can be overridden
    // base implementation
}
```

We'll cover virtual methods in Lesson 09.

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Return type | Goes before function name |
| `const &` | Const reference parameter |
| `const` method | Method doesn't modify object |
| `auto` | Type inference |
| Range-based for | `for (const auto &x : container)` |
| Bitwise flags | `flags & Flag::Value` |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript
function add(a: number, b: number): number { return a + b; }

// C++
int add(int a, int b) { return a + b; }
```

```typescript
// TypeScript
const isEnemy = (player: Player): boolean => player.team !== this.team;

// C++
bool Bot::isEnemy(const Player &player) const {
    return player.team != this->m_team;
}
```

## Common Mistakes

### Forgetting Return Type

```cpp
// Error - no return type
add(int a, int b) { return a + b; }

// Correct
int add(int a, int b) { return a + b; }
```

### Modifying in const Method

```cpp
int getCount() const {
    m_count++;  // Error! Can't modify in const method
    return m_count;
}
```

### Missing Semicolons

```cpp
if (x > 0)
    doSomething()   // Error! Missing semicolon
```

## What's Next

In Lesson 04, you'll learn about structs and how YaPB organizes game data:

- Struct vs class
- Memory layout
- The `BotTask` and `WeaponInfo` structures

**[Next: Lesson 04 - Structs & Data Organization](../04-structs-data/README.md)**
