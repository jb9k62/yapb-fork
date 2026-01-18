# Lesson 11: Entity System

Understand how GoldSrc handles game entities - players, weapons, doors, and everything else in the game world.

## Learning Objectives

- Understand `edict_t` and `entvars_t` structures
- Learn entity indexing and slots
- Iterate through entities safely
- Create a player listing function

## Theory: What is an Entity?

In GoldSrc, **everything** in the game world is an entity:
- Players (humans and bots)
- Weapons on the ground
- Doors and buttons
- The bomb (C4)
- Hostages
- Even invisible triggers

### TypeScript Equivalent Concept

```typescript
interface Entity {
    id: number;
    classname: string;
    position: Vector;
    health: number;
    // ... many more properties
}

const entities: (Entity | null)[] = new Array(MAX_ENTITIES);
```

### C++ GoldSrc Structure

```cpp
struct edict_t {
    int free;                 // Is this slot empty?
    int serialnumber;         // Unique ID for this entity
    entvars_t v;              // Entity variables (position, health, etc.)
    void* pvPrivateData;      // Game-specific data
    // ...
};
```

## Theory: edict_t vs entvars_t

### edict_t - The Container

```cpp
struct edict_t {
    int free;                 // 1 if slot is unused
    int serialnumber;         // Changes when entity is reused
    entvars_t v;              // The actual entity data
    void* pvPrivateData;      // Game DLL's private data
};
```

`edict_t` is the engine's view of an entity - a slot that can be filled or empty.

### entvars_t - The Data

```cpp
struct entvars_t {
    string_t classname;       // "player", "weapon_ak47", etc.
    Vector origin;            // Position in world
    Vector angles;            // Rotation
    Vector velocity;          // Movement speed
    float health;             // Health points
    float max_health;         // Maximum health
    int flags;                // FL_ONGROUND, FL_DUCKING, etc.
    int effects;              // Visual effects
    string_t netname;         // Player name
    int weapons;              // Weapon bitmask
    // ... 100+ more fields
};
```

`entvars_t` contains all the entity's properties.

### Accessing Entity Data

```cpp
edict_t* player = ...;

// Access via edict_t::v
float health = player->v.health;
Vector pos = player->v.origin;
const char* name = player->v.netname.chars();
const char* type = player->v.classname.chars();

// Check if entity is player
if (strcmp(player->v.classname.chars(), "player") == 0) {
    // It's a player
}
```

## Code Study: Entity Indexing

### File: `inc/engine.h:289-376` (Game class excerpt)

```cpp
// Get entity by index
edict_t* entityByIndex(int index) {
    return &m_startEntity[index];
}

// Get index of entity
int indexOfEntity(edict_t* ent) {
    return static_cast<int>(ent - m_startEntity);
}

// Get first entity (worldspawn, index 0)
edict_t* getStartEntity() {
    return m_startEntity;
}
```

### Entity Index Layout

```
Index  Entity
─────  ─────────────────────
0      Worldspawn (the map itself)
1      Player slot 1
2      Player slot 2
...
32     Player slot 32
33+    Other entities (weapons, items, etc.)
```

Players always occupy indices 1-32.

## Code Study: Client Array

### File: `inc/yapb.h:98-109`

YaPB maintains its own array of client info:

```cpp
struct Client {
   edict_t *ent {};                   // pointer to actual edict
   Vector origin {};                  // cached position
   int team {};                       // player team
   int flags {};                      // ClientFlags (Used, Alive, etc.)
   int radio {};                      // radio orders
   ClientNoise noise {};              // heard sounds
};
```

Accessed via `util.getClients()`.

## Code Study: Entity Iteration

### Iterating Players (Efficient)

```cpp
for (const auto &client : util.getClients()) {
    if (!(client.flags & ClientFlags::Used)) {
        continue;  // Slot not in use
    }
    if (!(client.flags & ClientFlags::Alive)) {
        continue;  // Player is dead
    }

    // Use client.ent, client.origin, etc.
    ctrl.msg("Player: %s at %.0f, %.0f, %.0f",
             client.ent->v.netname.chars(),
             client.origin.x, client.origin.y, client.origin.z);
}
```

### Iterating All Entities (Less Common)

```cpp
void iterateAllEntities() {
    edict_t* start = game.getStartEntity();
    int count = globals->maxEntities;

    for (int i = 0; i < count; i++) {
        edict_t* ent = &start[i];

        if (ent->free) continue;  // Slot is empty

        const char* classname = ent->v.classname.chars();
        ctrl.msg("Entity %d: %s", i, classname);
    }
}
```

### Using game.searchEntities

```cpp
// Find all weapons on ground
game.searchEntities("classname", "weaponbox", [](edict_t* ent) {
    ctrl.msg("Found weapon: %s", ent->v.model.chars());
    return EntitySearchResult::Continue;  // Keep searching
});
```

## Exercise: Create Player Listing Function

Create a function that lists all players with their status.

### Your Task

1. Create a `listPlayers()` function
2. Iterate through all connected players
3. Print name, team, health, and position
4. Handle bots vs humans differently

### Implementation

Add to `src/botlib.cpp` or create a utility function:

```cpp
void listPlayers() {
    ctrl.msg("=== Player List ===");

    int index = 0;
    for (const auto &client : util.getClients()) {
        if (!(client.flags & ClientFlags::Used)) {
            continue;
        }

        edict_t* ent = client.ent;
        if (!ent) continue;

        const char* name = ent->v.netname.chars();
        const char* status = (client.flags & ClientFlags::Alive) ? "Alive" : "Dead";
        const char* team = client.team == Team::Terrorist ? "T" : "CT";
        bool isBot = bots[ent] != nullptr;

        ctrl.msg("[%d] %s %s - %s (%.0f HP) at (%.0f, %.0f, %.0f)",
                 index++,
                 team,
                 isBot ? "[BOT]" : "[HUMAN]",
                 name,
                 ent->v.health,
                 client.origin.x,
                 client.origin.y,
                 client.origin.z);
    }

    ctrl.msg("Total: %d players", index);
}
```

### Add Console Command

In `src/control.cpp`:

```cpp
if (isCommand("players") || isCommand("list")) {
    listPlayers();
    return true;
}
```

### Build and Test

```bash
meson compile -C build
./deploy.sh
```

In game:
```
yb players
```

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: Entity Flags

### Common entvars_t Flags

```cpp
// ent->v.flags
FL_FLY          // Flying (noclip)
FL_SWIM         // Swimming
FL_ONGROUND     // Touching ground
FL_PARTIALGROUND // Near ground
FL_DUCKING      // Crouching
FL_GODMODE      // Invincible
FL_DORMANT      // Not active
FL_CLIENT       // Is a player
FL_FAKECLIENT   // Is a bot
```

### Checking Flags

```cpp
// Is player on ground?
bool onGround = (ent->v.flags & FL_ONGROUND) != 0;

// Is player a bot?
bool isBot = (ent->v.flags & FL_FAKECLIENT) != 0;

// Is player ducking?
bool ducking = (ent->v.flags & FL_DUCKING) != 0;
```

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| `edict_t` | Entity container/slot |
| `entvars_t` | Entity data (in `ent->v`) |
| Entity index | 0=world, 1-32=players, 33+=others |
| `FL_*` flags | Entity state flags |
| `ClientFlags` | YaPB's client state tracking |
| `util.getClients()` | Efficient player iteration |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript
interface Entity {
    classname: string;
    position: Vector;
    health: number;
}
const entities: Entity[];

// C++
edict_t entities[MAX_EDICTS];
// Access: entities[i].v.classname, entities[i].v.origin, etc.
```

```typescript
// TypeScript iteration
for (const entity of entities.filter(e => e.active)) {
    console.log(entity.name);
}

// C++
for (const auto &client : util.getClients()) {
    if (client.flags & ClientFlags::Used) {
        ctrl.msg("%s", client.ent->v.netname.chars());
    }
}
```

## What's Next

In Lesson 12, you'll learn about Vector Math:

- 3D vectors and operations
- Distance and dot product calculations
- Checking if an enemy is in front of the bot

**[Next: Lesson 12 - Vector Math & 3D Space](../12-vector-math/README.md)**
