# Lesson 05: Pointers & References

Understand memory addresses, pointers, and references - the core concept that differs most from TypeScript.

## Learning Objectives

- Understand memory addresses and dereferencing
- Learn the difference between pointers and references
- Master `nullptr` and null checking
- Understand `->` vs `.` operators
- Write safe code that handles null pointers

## Theory: The Fundamental Difference

### TypeScript: Everything is a Reference

```typescript
// Objects are always references
let bot1 = { health: 100 };
let bot2 = bot1;           // bot2 points to same object
bot2.health = 50;          // bot1.health is also 50 now!

// Primitives are values
let a = 10;
let b = a;                 // b is a copy
b = 20;                    // a is still 10
```

In TypeScript, you never think about this - the language handles it automatically.

### C++: You Choose

```cpp
// Value (copy)
Bot bot1;
Bot bot2 = bot1;           // bot2 is a COPY
bot2.health = 50;          // bot1.health unchanged

// Pointer (address)
Bot* ptr = &bot1;          // ptr holds address of bot1
ptr->health = 50;          // bot1.health is now 50

// Reference (alias)
Bot& ref = bot1;           // ref IS bot1 (alias)
ref.health = 50;           // bot1.health is now 50
```

In C++, you explicitly choose between value, pointer, and reference semantics.

## Theory: Pointers

### What is a Pointer?

A pointer is a variable that holds a memory address.

```cpp
int x = 42;
int* ptr = &x;    // ptr holds the ADDRESS of x

// Visualize memory:
// Address    Value    Variable
// 0x1000     42       x
// 0x1008     0x1000   ptr (holds address of x)
```

### Pointer Operations

```cpp
int x = 42;

int* ptr = &x;      // & = "address of" - get address
int value = *ptr;   // * = "dereference" - get value at address

*ptr = 100;         // Modify value through pointer
// x is now 100
```

### The `->` Operator

For pointers to structs/classes, `->` accesses members:

```cpp
Bot* bot = &someBot;

// These are equivalent:
bot->health = 100;
(*bot).health = 100;  // Dereference then access (awkward)
```

Think of `->` as "follow the pointer and access member."

## Theory: References

### What is a Reference?

A reference is an alias - another name for an existing variable.

```cpp
int x = 42;
int& ref = x;     // ref IS x (not a copy, not a pointer)

ref = 100;        // x is now 100
```

### References vs Pointers

| Aspect | Pointer | Reference |
|--------|---------|-----------|
| Syntax | `Type*` | `Type&` |
| Can be null | Yes | No (must be valid) |
| Can be reassigned | Yes | No |
| Access member | `ptr->member` | `ref.member` |
| Declaration | `Type* p = &x` | `Type& r = x` |

### When to Use Which

```cpp
// Use pointer when:
// - Value might be null
// - Need to reassign to different object
// - Need pointer arithmetic
edict_t* enemy = nullptr;  // May or may not have enemy

// Use reference when:
// - Value is always valid
// - Don't need to reassign
// - Want cleaner syntax
void processBot(Bot& bot) {  // bot always exists
    bot.health = 100;
}
```

## Theory: nullptr

### TypeScript null/undefined

```typescript
let enemy: Entity | null = null;
if (enemy !== null) {
    enemy.attack();
}
```

### C++ nullptr

```cpp
edict_t* enemy = nullptr;  // No enemy

if (enemy != nullptr) {
    // Safe to use enemy
    enemy->v.health -= 10;
}

// Shorthand (pointer is truthy when non-null)
if (enemy) {
    enemy->v.health -= 10;
}
```

### Danger: Dereferencing nullptr

```cpp
edict_t* enemy = nullptr;
enemy->v.health = 0;  // CRASH! Segmentation fault
```

This is the most common crash in C++ - always check before using pointers.

## Code Study: Bot's Enemy Pointer

### File: `inc/yapb.h:348-354`

```cpp
edict_t *m_pickupItem {};    // pointer to entity of item to pickup
edict_t *m_liftEntity {};    // pointer to lift entity
edict_t *m_breakableEntity {}; // pointer to breakable entity
edict_t *m_targetEntity {};  // entity bot is trying to reach
edict_t *m_avoidGrenade {};  // pointer to grenade to avoid
edict_t *m_hindrance {};     // the hindrance
edict_t *m_hearedEnemy {};   // the heard enemy
```

All initialized to `nullptr` via `{}`. These pointers track various entities the bot interacts with.

### File: `inc/yapb.h:721-725`

```cpp
edict_t *m_doubleJumpEntity {}; // entity that requested double jump
edict_t *m_radioEntity {};      // entity issuing radio command
edict_t *m_enemy {};            // pointer to enemy entity
edict_t *m_enemyBodyPartSet {}; // last enemy body part
edict_t *m_lastEnemy {};        // pointer to last enemy entity
edict_t *m_lastVictim {};       // pointer to killed entity
```

`m_enemy` is the bot's current target - could be `nullptr` if no enemy is visible.

## Code Study: Null Checking Pattern

### File: `src/botlib.cpp:71-78` (example pattern)

```cpp
bool Bot::seesEnemy(edict_t* player) {
    if (game.isNullEntity(player)) {
        return false;  // Safety check
    }

    // Now safe to access player->v
    Vector enemyPos = player->v.origin;
    // ...
}
```

The `game.isNullEntity()` helper checks both:
- Is the pointer null?
- Is the entity slot invalid?

### YaPB Null Check Helper

```cpp
// In Game class
bool isNullEntity(edict_t* ent) const {
    return !ent || ent->free || !ent->pvPrivateData;
}
```

This is safer than just `ent != nullptr` because entities can become invalid.

## Exercise: Write Safe Enemy Info Printer

Create a function that safely prints information about the bot's current enemy.

### Your Task

1. Add a `printEnemyInfo()` method to Bot
2. Safely check if enemy exists
3. Print enemy name, health, and position
4. Handle the case when there's no enemy

### Step-by-Step

#### Step 1: Add Declaration

In `inc/yapb.h`, add to Bot's public methods:

```cpp
void printEnemyInfo();
```

#### Step 2: Implement with Safety Checks

In `src/combat.cpp`, add:

```cpp
void Bot::printEnemyInfo() {
    // Check if we have an enemy
    if (game.isNullEntity(m_enemy)) {
        ctrl.msg("%s: No enemy targeted", pev->netname.chars());
        return;
    }

    // Safe to access m_enemy now
    entvars_t* enemy = &m_enemy->v;

    ctrl.msg("%s targeting %s - HP: %.0f, Pos: (%.0f, %.0f, %.0f)",
             pev->netname.chars(),        // Our name
             enemy->netname.chars(),      // Enemy name
             enemy->health,               // Enemy health
             enemy->origin.x,             // Position X
             enemy->origin.y,             // Position Y
             enemy->origin.z);            // Position Z
}
```

#### Step 3: Call from Somewhere

In `src/combat.cpp`, find the combat logic and add a call (or create a console command).

### Build and Test

```bash
meson compile -C build
./deploy.sh
```

During combat, the function should print enemy info when called.

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: edict_t and entvars_t

### The Entity Structure

```cpp
struct edict_t {
    int free;                    // Is this slot free?
    int serialnumber;            // Unique ID
    entvars_t v;                 // Entity variables
    void* pvPrivateData;         // Game-specific data
    // ...
};

struct entvars_t {
    string_t classname;          // "player", "weapon_ak47", etc.
    Vector origin;               // Position
    Vector angles;               // Rotation
    float health;                // Health points
    int flags;                   // FL_ONGROUND, FL_DUCKING, etc.
    string_t netname;            // Player name
    // ... many more fields
};
```

### Accessing Entity Data

```cpp
edict_t* player = ...;

// Access via edict_t::v
float health = player->v.health;
Vector pos = player->v.origin;
const char* name = player->v.netname.chars();

// Bot class has pev shortcut
// pev = &ent()->v
float myHealth = pev->health;  // Same as ent()->v.health
```

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| `*` (pointer) | Variable holding memory address |
| `&` (address of) | Get address of a variable |
| `*` (dereference) | Get value at address |
| `&` (reference) | Alias for existing variable |
| `->` | Access member through pointer |
| `nullptr` | Null pointer value |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript
let enemy: Entity | null = null;
if (enemy !== null) {
    console.log(enemy.health);
}

// C++
edict_t* enemy = nullptr;
if (enemy != nullptr) {  // or just: if (enemy)
    ctrl.msg("Health: %f", enemy->v.health);
}
```

```typescript
// TypeScript (pass by reference automatic for objects)
function damage(entity: Entity) {
    entity.health -= 10;  // Modifies original
}

// C++ (explicit reference)
void damage(edict_t& entity) {
    entity.v.health -= 10;  // Modifies original
}

// C++ (pointer version)
void damage(edict_t* entity) {
    if (entity) {
        entity->v.health -= 10;
    }
}
```

## Common Mistakes

### Forgetting Null Check

```cpp
// DANGEROUS
void attack(edict_t* enemy) {
    enemy->v.health -= 10;  // Crash if enemy is null!
}

// SAFE
void attack(edict_t* enemy) {
    if (!enemy) return;
    enemy->v.health -= 10;
}
```

### Using `.` with Pointers

```cpp
edict_t* enemy = ...;

enemy.v.health;   // ERROR! '.' doesn't work with pointers
enemy->v.health;  // Correct
```

### Dangling References

```cpp
int& getRef() {
    int local = 42;
    return local;    // DANGER! local destroyed after return
}
// Reference now points to invalid memory
```

## What's Next

In Lesson 06, you'll learn about smart pointers and RAII:

- Stack vs heap allocation
- `UniquePtr` for automatic cleanup
- RAII (Resource Acquisition Is Initialization)
- How YaPB manages bot memory

**[Next: Lesson 06 - Smart Pointers & RAII](../06-smart-pointers-raii/README.md)**
