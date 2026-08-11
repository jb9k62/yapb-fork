# Lesson 00: Setup — install Zig 0.16.0

Get the toolchain green before writing any code. Ends with you compiling and running your first Zig program.

## Learning Objectives

- Install **exactly Zig 0.16.0**
- Understand the project layout
- Cross-compile a 32-bit binary *before* the game needs it
- Run `.learn-zig/reference/` sanity material

## Step 1: Install Zig 0.16.0 (exact version)

Zig's stdlib API changes between minor releases. **This whole curriculum is pinned to 0.16.0.** A different version will break the snippets.

```bash
# Check if you already have zig
zig version

# Expected: 0.16.0   (if a different version, get the 0.16.0 tarball)
```

Install from https://ziglang.org/download/ — grab the **0.16.0** tarball for your platform, unpack it, and put it on your `PATH`:

```bash
# Example (adjust arch to your machine: x86_64, aarch64, ...)
cd ~/bin
curl -LO https://ziglang.org/download/0.16.0/zig-x86_64-linux-0.16.0.tar.xz
tar -xf zig-x86_64-linux-0.16.0.tar.xz
# add to your shell profile:
export PATH="$HOME/bin/zig-x86_64-linux-0.16.0:$PATH"
```

Verify:

```bash
zig version   # -> 0.16.0
zig env       # -> JSON of the toolchain config (worth a glance)
```

> **If you can't get exactly 0.16.0:** tell me and I'll adjust the reference API file. Don't try to "just use a version that's close" — the errors will be confusing.

## Step 2: Why a 32-bit target matters (read this once)

CS 1.6's dedicated server `hlds_linux` is a **32-bit ELF binary**. A plugin loaded into it must also be 32-bit. That's a cross-compile from your (presumably 64-bit) machine.

Where C needs `gcc-multilib` and friends, **Zig cross-compiles with zero extra packages** — it ships its own libc for every target. That's a real Zig superpower and it's why this is the perfect tool for this job.

We won't use it until Step 5, but keep the command in mind:

```bash
zig build -Dtarget=x86-linux-gnu
```

## Step 3: Project layout

```text
.learn-zig/
├── README.md                     <- index (start here)
├── reference/
│   ├── zig-0_16-api.md           <- pinned API cheatsheet (keep open!)
│   ├── zig-cheatsheet.md         <- TS/C#/Java/Python -> Zig
│   ├── goldsrc-plugin-abi.md     <- how plugin hooks work
│   └── enginefuncs.zig           <- the engine table we transcribe
├── 00-setup/   <- you are here
├── 01-hello/
├── 02-guess-cli/
├── 03-cimport/
├── 04-extern-abi/
├── 05-metamod-skeleton/
├── 06-chat-hook/
├── 07-guess-ingame/
└── 08-whereami/
```

Each lesson is a folder with a `README.md` (this file's format) and sometimes a `solution/` subfolder **you should not open until you're stuck**.

## Optional headphones-on: read the pinned API sheet

Open `reference/zig-0_16-api.md`. You don't need to memorize it — but notice that **Zig 0.16 changed the entry point and the I/O API** compared to older tutorials you'll find online. If a tutorial's `main` doesn't take `init: std.process.Init`, it's for an older Zig.

## Step 4: Sanity run

Create a scratch file anywhere and run it:

```zig
// /tmp/hello.zig
const std = @import("std");

pub fn main(init: std.process.Init) !void {
    _ = init;
    std.debug.print("Hello from Zig {s}!\n", .{ @import("builtin").zig_version_string });
}
```

```bash
zig run /tmp/hello.zig
# Hello from Zig 0.16.0!
```

If that prints, your toolchain is ready.

## Checkpoint

- [ ] `zig version` prints **0.16.0**
- [ ] `zig run /tmp/hello.zig` prints the greeting
- [ ] You've skimmed `reference/zig-0_16-api.md` (the "entry point" and "build system" sections)
- [ ] You understand *why* the plugin will need the 32-bit target

## What's Next

Lesson 01 — write a small Zig program that reads your name and greets you, learning the core syntax you'll reuse in the `!guess` game.

**[Next: Lesson 01 — Hello & core Zig](../01-hello/README.md)**
