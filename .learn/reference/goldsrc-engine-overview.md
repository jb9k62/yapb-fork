# GoldSrc Engine Overview

Understanding the Half-Life engine architecture for bot development.

## Engine Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    GoldSrc Engine                        │
│  (hl.exe / hl.so)                                       │
│                                                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │   Physics   │  │  Rendering  │  │  Networking │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
│                                                          │
│  ┌─────────────────────────────────────────────────┐   │
│  │              Engine Functions (engfuncs)         │   │
│  │  pfnServerPrint, pfnCreateEntity, etc.          │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────┬───────────────────────────────┘
                          │
                          │ Loads via liblist.gam
                          ▼
┌─────────────────────────────────────────────────────────┐
│                     YaPB (yapb.so)                       │
│                                                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │  Bot Logic  │  │ Navigation  │  │   Combat    │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
│                                                          │
│  ┌─────────────────────────────────────────────────┐   │
│  │              Game DLL Functions                  │   │
│  │  GetEntityAPI, GiveFnptrsToDll, etc.            │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────┬───────────────────────────────┘
                          │
                          │ Loads internally
                          ▼
┌─────────────────────────────────────────────────────────┐
│               Counter-Strike (cs.so)                     │
│                                                          │
│  Original game logic, weapons, gamerules               │
└─────────────────────────────────────────────────────────┘
```

## Plugin Loading Process

### 1. Engine Starts
```
1. Engine reads liblist.gam
2. Finds: gamedll_linux "addons/yapb/yapb.so"
3. Loads yapb.so as the "game DLL"
```

### 2. GiveFnptrsToDll Called
```cpp
// First function called by engine
// File: src/linkage.cpp:1072
GiveFnptrsToDll(enginefuncs_t *table, globalvars_t *glob)
{
    // Receive engine function pointers
    memcpy(&engfuncs, table, sizeof(enginefuncs_t));
    globals = glob;

    // YaPB then loads the real game DLL (cs.so)
    // and forwards calls appropriately
}
```

### 3. GetEntityAPI Called
```cpp
// Engine requests game functions
// File: src/linkage.cpp:76
GetEntityAPI(gamefuncs_t *table, int version)
{
    // YaPB provides its own function table
    // with hooks for game events
    table->pfnStartFrame = [](){ bots.frame(); };
    table->pfnClientConnect = [](){ /* track players */ };
    // ...
}
```

## Key Engine Callbacks

| Callback | When Called | YaPB Use |
|----------|-------------|----------|
| `GiveFnptrsToDll` | Plugin load | Receive engine API |
| `GameInit` | Server init | Register cvars |
| `ServerActivate` | Map load | Load navigation graph |
| `StartFrame` | Every frame | Bot AI execution |
| `ClientConnect` | Player joins | Track new players |
| `ClientDisconnect` | Player leaves | Cleanup |
| `ClientCommand` | Player command | Handle bot commands |

## Engine Functions (engfuncs)

After `GiveFnptrsToDll`, YaPB can call engine functions:

### Essential Functions

```cpp
// Print to server console
engfuncs.pfnServerPrint("Message\n");

// Create an entity
edict_t* ent = engfuncs.pfnCreateEntity();

// Get entity by index
edict_t* ent = engfuncs.pfnPEntityOfEntIndex(index);

// Make player execute command
engfuncs.pfnClientCommand(ent, "say hello");

// Trace line (raycast)
engfuncs.pfnTraceLine(start, end, flags, ignore, &result);

// Precache resources
engfuncs.pfnPrecacheModel("models/player.mdl");
engfuncs.pfnPrecacheSound("weapons/ak47-1.wav");
```

### Message System

```cpp
// Send network messages to clients
engfuncs.pfnMessageBegin(MSG_ONE, msgType, nullptr, client);
engfuncs.pfnWriteByte(value);
engfuncs.pfnWriteString("text");
engfuncs.pfnMessageEnd();
```

## Entity System

### Entity Structure

```cpp
struct edict_t {
    int free;           // Is slot empty?
    int serialnumber;   // Unique ID
    entvars_t v;        // Entity data
    void* pvPrivateData; // Game-specific data
};
```

### Entity Data (entvars_t)

```cpp
struct entvars_t {
    // Identity
    string_t classname;     // "player", "weapon_ak47"
    string_t netname;       // Player name

    // Transform
    Vector origin;          // Position
    Vector angles;          // Rotation
    Vector velocity;        // Movement

    // State
    float health;           // Current health
    float max_health;       // Maximum health
    float armorvalue;       // Armor
    int flags;              // FL_ONGROUND, FL_DUCKING, etc.
    int effects;            // Visual effects

    // Combat
    int weapons;            // Weapon bitmask
    int button;             // Current buttons pressed
    int oldbuttons;         // Previous buttons

    // And 100+ more fields...
};
```

### Entity Indices

```
Index 0:    Worldspawn (the map itself)
Index 1-32: Player slots (humans and bots)
Index 33+:  Dynamic entities (weapons, items, etc.)
```

### Common Entity Flags

```cpp
FL_FLY          // Flying (noclip mode)
FL_SWIM         // In water
FL_ONGROUND     // Touching ground
FL_DUCKING      // Crouching
FL_GODMODE      // Invincible
FL_DORMANT      // Inactive
FL_CLIENT       // Is a player
FL_FAKECLIENT   // Is a bot
FL_MONSTER      // Is an NPC
```

## Global Variables (globals)

```cpp
globalvars_t* globals;

// Access common values
float time = globals->time;          // Current game time
float frametime = globals->frametime; // Time since last frame
int maxEntities = globals->maxEntities;
int maxClients = globals->maxClients;
Vector v_forward = globals->v_forward; // Forward vector
```

## The Frame Loop

```
┌─────────────────────────────────────────┐
│               StartFrame()              │
│                                         │
│  1. Update game time                    │
│  2. Process physics                     │
│  3. Call pfnStartFrame in game DLL     │
│     └── YaPB: bots.frame()             │
│         └── For each bot: bot->frame() │
│  4. Process networking                  │
│  5. Render (client only)               │
│                                         │
│  Repeats at sv_fps rate (default 100)  │
└─────────────────────────────────────────┘
```

## MetaMod Integration

YaPB can run standalone or under MetaMod.

### Standalone Mode
```
Engine → YaPB → CS Game DLL
```

### MetaMod Mode
```
Engine → MetaMod → YaPB → CS Game DLL
                └→ Other plugins
```

MetaMod provides additional hooks and allows multiple plugins.

## Console Variables (CVars)

### Registering CVars

```cpp
// In YaPB
ConVar cv_quota("yb_quota", "10", "Number of bots to maintain");

// Usage
int quota = cv_quota.as<int>();
```

### Engine CVars

```cpp
// Access engine/game cvars
extern ConVar mp_freezetime;
extern ConVar mp_roundtime;
extern ConVar sv_gravity;

float freezeTime = mp_freezetime.as<float>();
```

## Bot Creation

### The Process

```cpp
// 1. Request bot creation from engine
edict_t* ent = engfuncs.pfnCreateFakeClient("BotName");

// 2. Engine assigns entity slot (1-32)
// 3. Bot appears in player list

// 4. YaPB creates Bot object
auto bot = cr::makeUnique<Bot>(ent, difficulty, ...);

// 5. Bot thinks every frame
void Bot::frame() {
    // AI logic here
}
```

### Bot Input Simulation

```cpp
// Bots don't have real input - we simulate it
void Bot::runMovement() {
    // Set movement intention
    pev->button = IN_FORWARD;  // Press forward
    if (shouldJump) {
        pev->button |= IN_JUMP;
    }

    // Engine handles actual movement physics
}
```

## Useful Resources

### Source Code References
- `ext/linkage/goldsrc.h` - Engine structures
- `src/linkage.cpp` - Plugin interface
- `inc/engine.h` - Game class wrapper

### External Documentation
- HLSDK (Half-Life SDK) documentation
- Metamod plugin development guide
- Valve Developer Community wiki

## Common Tasks

### Find Nearest Player

```cpp
edict_t* findNearest(const Vector& origin) {
    edict_t* nearest = nullptr;
    float minDist = kInfiniteDistance;

    for (int i = 1; i <= globals->maxClients; i++) {
        edict_t* ent = engfuncs.pfnPEntityOfEntIndex(i);
        if (!ent || ent->free) continue;
        if (ent->v.health <= 0) continue;

        float dist = (ent->v.origin - origin).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = ent;
        }
    }
    return nearest;
}
```

### Trace Line (Raycast)

```cpp
bool canSee(const Vector& from, const Vector& to) {
    TraceResult tr;
    engfuncs.pfnTraceLine(from, to, IGNORE_MONSTERS, nullptr, &tr);
    return tr.flFraction >= 1.0f;  // 1.0 = nothing blocked
}
```

### Get Entity Class

```cpp
bool isWeapon(edict_t* ent) {
    return strncmp(ent->v.classname.chars(), "weapon_", 7) == 0;
}

bool isPlayer(edict_t* ent) {
    return strcmp(ent->v.classname.chars(), "player") == 0;
}
```
