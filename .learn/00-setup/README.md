# Lesson 00: Setup

Before diving into code, let's set up your development environment and verify everything works.

## Learning Objectives

- Install required build tools
- Successfully compile YaPB
- Install the bot into Counter-Strike 1.6
- Verify the bot loads correctly

## Prerequisites

### Counter-Strike 1.6

You need CS 1.6 installed. The Steam version is recommended:

```bash
# Find your CS 1.6 installation
# Typical Steam path on Linux:
ls ~/.steam/steam/steamapps/common/Half-Life/

# You should see:
# cstrike/  (this is CS 1.6)
# valve/    (original Half-Life)
```

If you don't have CS 1.6, purchase it on Steam or use the Xash3D engine (free, open-source implementation).

### Build Tools

Install the required compilers and build system:

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install clang lld meson ninja-build git

# Verify installations
clang --version    # Should show Clang 14+
meson --version    # Should show 0.60+
ninja --version    # Should show 1.10+
```

For 32-bit builds (required for CS 1.6), you also need:

```bash
# Ubuntu/Debian - 32-bit libraries
sudo apt install gcc-multilib g++-multilib
```

## Step 1: Build YaPB

From the repository root:

```bash
# Navigate to YaPB directory
cd /home/jordan/Code/oss/yapb

# Set up the build (32-bit by default)
meson setup build

# Compile
meson compile -C build

# Verify the output
ls -la build/yapb.so
```

You should see `yapb.so` (about 1-2MB).

### Build Options

| Option | Description | Example |
|--------|-------------|---------|
| Default | 32-bit release build | `meson setup build` |
| 64-bit | For 64-bit engines | `meson setup build-amd64 -D64bit=true` |
| Debug | With debug symbols | `meson setup build-debug --buildtype=debug` |
| Native | Optimized for your CPU | `meson setup build-native -Dnative=true` |

For this curriculum, use the default 32-bit build.

## Step 2: Install in CS 1.6

### 2.1 Copy Files

```bash
# Set your CS 1.6 path (adjust as needed)
CS_DIR=~/.steam/steam/steamapps/common/Half-Life/cstrike

# Create the addon directory
mkdir -p "$CS_DIR/addons/yapb"

# Copy the compiled bot
cp build/yapb.so "$CS_DIR/addons/yapb/"

# Copy configuration files
cp -r cfg/addons/yapb/conf "$CS_DIR/addons/yapb/"
```

### 2.2 Configure liblist.gam

The `liblist.gam` file tells the engine which DLL to load. Edit it:

```bash
# Open in your editor
nano "$CS_DIR/liblist.gam"
```

Find this line:
```
gamedll_linux "dlls/cs.so"
```

Change it to:
```
gamedll_linux "addons/yapb/yapb.so"
```

This tells CS to load YaPB first, which then loads the real game DLL.

### 2.3 Understand the Plugin Chain

```
Engine (hl.so)
    │
    ▼
YaPB (yapb.so)          ◄── Your bot code runs here
    │
    ▼
Counter-Strike (cs.so)  ◄── Original game
```

YaPB intercepts engine calls, does its bot logic, then passes calls to the real game.

## Step 3: Test the Installation

### Launch CS 1.6

```bash
# Launch via Steam
steam steam://rungameid/10

# Or directly (adjust path)
~/.steam/steam/steamapps/common/Half-Life/hl.sh -game cstrike
```

### Open the Console

Press `~` (tilde) to open the game console. You should see:

```
[YAPB] YaPB vX.X.X (Build: XXX)
[YAPB] By YaPB Project Developers
```

If you see this, the bot loaded successfully.

### Add Bots

In the console, type:

```
yb add
yb add
yb add
```

Start a map:
```
map de_dust2
```

You should see bots spawning and playing.

### Useful Console Commands

| Command | Description |
|---------|-------------|
| `yb add` | Add one bot |
| `yb add 5` | Add 5 bots |
| `yb kick` | Kick one bot |
| `yb kick all` | Kick all bots |
| `yb quota 10` | Set bot count to 10 |
| `yb help` | List all commands |

## Step 4: Development Workflow

Now that everything works, here's your development workflow:

### Make a Change

```bash
# Edit a source file
nano src/linkage.cpp

# Add a test message (we'll do this in Lesson 01)
```

### Rebuild

```bash
# Recompile (only changed files)
meson compile -C build
```

### Deploy

```bash
# Copy new .so to game
cp build/yapb.so "$CS_DIR/addons/yapb/"
```

### Test

```bash
# Restart CS 1.6 (DLL is loaded at startup, not on map change)
# Then test your changes in-game
```

### Quick Deploy Script

Create a helper script to speed up iteration:

```bash
# Create deploy.sh in the yapb directory
cat > deploy.sh << 'EOF'
#!/bin/bash
CS_DIR=~/.steam/steam/steamapps/common/Half-Life/cstrike
meson compile -C build && cp build/yapb.so "$CS_DIR/addons/yapb/"
echo "Deployed! Restart CS 1.6 to test."
EOF

chmod +x deploy.sh

# Now you can just run:
./deploy.sh
```

## Troubleshooting

### "Library not found" Error

```bash
# Check the file exists
ls -la "$CS_DIR/addons/yapb/yapb.so"

# Check it's a valid 32-bit binary
file "$CS_DIR/addons/yapb/yapb.so"
# Should say: ELF 32-bit LSB shared object
```

### Build Fails with Missing Headers

```bash
# Ensure submodules are initialized
git submodule update --init --recursive
```

### Game Crashes on Startup

1. Check the console for error messages
2. Verify `liblist.gam` path is correct
3. Try reverting to original: `gamedll_linux "dlls/cs.so"`

### 64-bit vs 32-bit Mismatch

CS 1.6 is a 32-bit game. If you built 64-bit:

```bash
# Clean and rebuild for 32-bit
rm -rf build
meson setup build
meson compile -C build
```

## Reverting Changes

To restore original game (no bot):

```bash
# Edit liblist.gam
nano "$CS_DIR/liblist.gam"

# Change back to:
gamedll_linux "dlls/cs.so"
```

## Checkpoint

Before moving on, verify:

- [ ] YaPB compiles without errors
- [ ] `yapb.so` is copied to `addons/yapb/`
- [ ] `liblist.gam` points to `addons/yapb/yapb.so`
- [ ] CS 1.6 shows YaPB startup message in console
- [ ] `yb add` creates a bot in-game

## What's Next

You're ready to write your first code change. In Lesson 01, you'll add a custom startup message and learn about:

- How C++ compilation works (vs TypeScript transpilation)
- The `#include` directive
- The entry point function `GiveFnptrsToDll`

**[Next: Lesson 01 - Hello World](../01-hello-world/README.md)**
