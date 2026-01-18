# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

YaPB (Yet Another POD Bot) is an AI bot for Counter-Strike 1.6 and Counter-Strike: Condition Zero. It provides computer-controlled players that allow playing without connecting to game servers.

## Build Commands

**Primary build system: Meson + Ninja**

```bash
# Setup and build (Linux 32-bit with Clang - default)
meson setup build
meson compile -C build

# Linux 64-bit
meson setup build-amd64 -D64bit=true
meson compile -C build-amd64

# Native optimized build (uses -march=native)
meson setup build-native -Dnative=true
meson compile -C build-native

# Debug build
meson setup build-debug --buildtype=debug
meson compile -C build-debug

# Cross-compile for Windows (requires cross-file)
meson setup build-win --cross-file windows-x86-clang
meson compile -C build-win
```

**Build options** (set with `-Doption=value`):
- `64bit` - Build as 64-bit binary (default: false)
- `native` - Build for native machine with march=native (default: false)
- `winxp` - Windows XP compatibility for MSVC (default: false)
- `nosimd` - Disable SSE/NEON optimizations (default: false)
- `static_linkent` - Use static entity linking instead of dlsym hooking (default: false)

**Alternative: CMake**
```bash
mkdir build && cd build
cmake ..
make
```

**Output:** Shared library named `yapb.so` (Linux), `yapb.dll` (Windows), or `yapb.dylib` (macOS). For 64-bit builds: `yapb_amd64.*`

## Installation (Testing Your Build)

After building, you need to tell Counter-Strike to load your `.so` file.

**Step 1: Copy files to game directory**
```bash
# Assuming CS is at ~/.steam/steam/steamapps/common/Half-Life/cstrike/
CS_DIR=~/.steam/steam/steamapps/common/Half-Life/cstrike

# Create addon directory and copy the built .so
mkdir -p "$CS_DIR/addons/yapb"
cp build/yapb.so "$CS_DIR/addons/yapb/"

# Copy config files
cp -r cfg/addons/yapb/conf "$CS_DIR/addons/yapb/"
```

**Step 2: Edit liblist.gam to load your bot**

Edit `$CS_DIR/liblist.gam` and change the `gamedll_linux` line:
```
// Original (loads the normal game):
gamedll_linux "dlls/cs.so"

// Changed (loads your bot, which then loads cs.so):
gamedll_linux "addons/yapb/yapb.so"
```

**Step 3: Run the game and check console**
```bash
# Launch CS 1.6, open console with ~ key
# You should see: "*** HELLO FROM YAPB! Map is loading! ***"
```

**To revert:** Change `liblist.gam` back to `gamedll_linux "dlls/cs.so"`

## How the Plugin System Works

```
┌────────────────────────────────────────────────────────────────┐
│ Counter-Strike 1.6 (GoldSrc Engine)                            │
│                                                                │
│  1. Game reads liblist.gam                                     │
│  2. Loads gamedll_linux "addons/yapb/yapb.so"                  │
│  3. Calls GiveFnptrsToDll() - passes engine API to your code   │
│  4. Your bot saves the API (engfuncs) and loads the real cs.so │
│  5. On map load: ServerActivate() is called                    │
│  6. Your bot hooks into game events (player spawn, damage, etc)│
└────────────────────────────────────────────────────────────────┘
```

**Key engine functions** (in `engfuncs` after GiveFnptrsToDll):
- `engfuncs.pfnServerPrint(msg)` - Print to game console
- `engfuncs.pfnCreateEntity()` - Spawn entities
- `engfuncs.pfnClientCommand(ent, cmd)` - Make a player run a command

**Key lifecycle callbacks** (in `src/linkage.cpp`):
- `GiveFnptrsToDll` - First function called, receive engine API
- `ServerActivate` - Map loaded, server running, safe to use engine functions
- `StartFrame` - Called every game frame (~100fps)
- `ClientConnect/Disconnect` - Player join/leave

## Architecture

### Core Subsystems (Singleton Pattern)

The codebase uses singleton classes for major subsystems, accessed via global instances:

- **`Bot`** (`inc/yapb.h`) - Main bot AI class with combat, navigation, and task execution logic
- **`BotManager`** (`inc/manager.h`) - Creates and coordinates all bots. Global: `bots`
- **`BotGraph`** (`inc/graph.h`) - Navigation graph (waypoint) system. Global: `graph`
- **`PathPlanner`** (`inc/planner.h`) - A* pathfinding. Global: `planner`
- **`Game`** (`inc/engine.h`) - Game state and engine integration. Global: `game`
- **`BotConfig`** (`inc/config.h`) - Configuration loading. Global: `conf`
- **`BotControl`** (`inc/control.h`) - Console command interface. Global: `ctrl`

### Bot AI Flow

1. **`Bot::logic()`** - Frame-skippable operations
2. **`Bot::frame()`** - Per-frame execution calling `update()` -> `executeTasks()`
3. **Task system** - Priority-based tasks in `m_tasks` array (normal_, camp_, attackEnemy_, etc.)
4. **Navigation** - `findPath()` uses A* via `PathPlanner`, path stored in `m_pathWalk`

### Navigation Graph

- Nodes stored in `BotGraph::m_paths` (max 4096 nodes, 8 links each)
- `Path` struct: position, flags (Ladder, Camp, Crouch, etc.), links to other nodes
- Spatial lookup via `m_hashTable` bucket system
- Graph files: `.graph` (YaPB format) or `.pwf` (legacy PODBot format)

### Key Source Files

- `src/navigate.cpp` - Pathfinding and movement (largest module)
- `src/combat.cpp` - Weapon selection and combat logic
- `src/tasks.cpp` - Task execution (normal_, attackEnemy_, camp_, etc.)
- `src/planner.cpp` - A* algorithm implementation
- `src/manager.cpp` - Bot lifecycle and quota management
- `src/graph.cpp` - Navigation graph loading/saving/editing

### External Dependencies (Git Submodules)

- `ext/crlib` - Custom C++ runtime library (containers, SIMD, threading)
- `ext/linkage` - GoldSrc engine and MetaMod API bindings

## Code Conventions

- C++17 standard with no exceptions (`-fno-exceptions`) and no RTTI (`-fno-rtti`)
- `cr::` namespace from crlib for utilities (strings, containers, etc.)
- Console variables: `cv_*` prefix (e.g., `cv_debug`, `cv_quota`)
- Game cvars: `mp_*` prefix (e.g., `mp_freezetime`)
- Task methods: `task_()` suffix (e.g., `normal_()`, `camp_()`)
