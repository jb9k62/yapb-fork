# Zig 0.16.0 API Reference

> Target: **Zig 0.16.0** (released 2026-04-13). Zig's stdlib changes *a lot* between versions. If you get compile errors, the #1 cause is you're on a different Zig. Verify with `zig version`.

## Install the exact version

```bash
# Find your OS package, or use the official tarball:
# https://ziglang.org/download/  -> 0.16.0
zig version   # must print 0.16.0
```

## The entry point changed (0.15 → 0.16)

In 0.16 the process-related state is passed in, and I/O moved to a new `std.Io` namespace:

```zig
const std = @import("std");

// 0.16 style — receives process state
pub fn main(init: std.process.Init) !void {
    const io = init.io;

    var buf: [4096]u8 = undefined;
    var writer = std.Io.File.stdout().writerStreaming(io, &buf);
    var stdout = &writer.interface;

    try stdout.writeAll("Hello, CS 1.6!\n");
    try writer.flush();
}
```

The old `std.io.getStdOut()` / `std.io.getStdIn()` API from ≤0.14 is **gone** in 0.16. Use `std.Io.File.stdout()`, `.stdin()`, `.stderr()`. Each needs a `writerStreaming(io, &buf)` / `readerStreaming(io, &buf)` and a buffer you own.

> Less common but valid: `pub fn main() void` and `std.debug.print("...\n", .{})` still work for quick prints (they don't touch process I/O). Use those in tiny experiments; use `std.Io` when you actually read stdin or write to stdout.

## Printing / formatting

```zig
// simple debug print (no process I/O needed)
std.debug.print("value = {d}\n", .{42});

// via the 0.16 Io writer
var buf: [4096]u8 = undefined;
var writer = std.Io.File.stdout().writerStreaming(io, &buf);
var stdout = &writer.interface;
try stdout.print("You guessed {d}. Did: {s}\n", .{ 42, "too low" });
try writer.flush();
```

Format specifiers: `{d}` int, `{s}` string slice, `{x}` hex, `{d:.2}` float with 2 decimals.

To build a string into a buffer without writing: `std.fmt.bufPrint(&dest, "fmt", .{...})` returns `![]u8`.

## Reading lines from stdin

```zig
var buf: [4096]u8 = undefined;
var reader = std.Io.File.stdin().readerStreaming(io, &buf);
var stdin = &reader.interface;

while (try stdin.takeDelimiter('\n')) |line| {
    // line: []const u8, the trimmed contents up to (not including) '\n'
    std.debug.print("got: {s}\n", .{line});
}
```

- `stdin.takeDelimiter(delim)` returns `!?[]const u8` — `null` on end of stream.
- Every `takeDelimiter` inside the loop reads from the *same* user buffer, so the previous `line` is invalidated on the next call.

## Random numbers

```zig
var prng = std.Random.DefaultPrng.init(seed); // seed: u64 (try @intCast(std.time.nanoTimestamp()))
const rand = prng.random();

const n = rand.int(u8);                       // 0..=255
const in_range = rand.intRangeAtMost(u8, 1, 100); // 1..=100 inclusive
```

- `DefaultPrng` (Xoroshiro128) is fine for games; it is **not** cryptographically secure.
- Seed from `std.crypto.random.int(u64)` or the wall clock.

## Slices vs arrays

```zig
const arr: [4]u8 = .{ 1, 2, 3, 4 };        // fixed-size array
const sl: []const u8 = &arr;               // slice = ptr + length
const view = "hello"[0..2];                // "he"
```

Slice literals `.{...}` vs array literals `[_]T{...}`. A `[]const u8` is *the* Zig string type.

## Optionals and error unions

```zig
var maybe: ?u32 = null;      // optional — can be null
var result: !u32 = 42;       // error union — value or error

// catching null: `orelse`
const v = maybe orelse 0;

// catching errors: `try` inside a `!` function, or `catch`
const v2 = result catch 0;
```

`!T` is pronounced "T or an error." `void` returning functions that can fail are `!void`.

## Control flow

```zig
for (items) |item| { ... }              // iterate arrays/slices
for (items, 0..) |item, i| { ... }      // with index
for (kv.items()) |k, v| { ... }         // hash map

while (cond) : (continue) { ... }       // with continue expression
while (maybe) |val| { ... }             // unwrap optional
while (try stream.takeDelimiter('\n')) |line| { ... }

if (cond) { } else if { } else { }
switch (thing) { ... }                  // exhaustive, no fallthrough

// scoped exit cleanup — runs when the block exits (happy or error path)
defer std.debug.print("cleaning up\n", .{});
```

`while |capture|` auto-unwraps optionals — inside the block the value is non-null.

## Build system (0.16)

`zig init` gives you a `build.zig`. A minimal **executable**:

```zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "app",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });
    b.installArtifact(exe);
}
```

Build: `zig build` (outputs to `zig-out/bin/`) or run directly with `zig run src/main.zig` / `zig build run`.

A **shared library** (dynamic) — what we need for the game plugin:

```zig
const lib = b.addLibrary(.{
    .linkage = .dynamic,               // produces .so
    .name = "csguess",                 // -> libcsguess.so
    .root_module = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    }),
});
b.installArtifact(lib);
```

### Cross-compiling to 32-bit (CRITICAL for CS 1.6)

CS 1.6's dedicated server (`hlds_linux`) is a **32-bit** binary. The plugin must be 32-bit.

```bash
zig build -Dtarget=x86-linux-gnu
```

That's it — Zig bundles the 32-bit libc and no extra system packages (unlike C, where you'd need `gcc-multilib`). This is a genuinely cool Zig superpower: producing a binary for a machine you aren't running on.

> **Plugin rule of thumb:** the game plugin `.so` never uses `main(init: std.process.Init)` (you're not a process, you're a library loaded into one), and stays away from `std.Io`'s stdin/stdout. It talks to the game through raw function pointers instead — the subject of Steps 4–5.

## Common compile errors in 0.16

| Error / symptom | Cause | Fix |
|---|---|---|
| `use of undeclared identifier 'getStdIn'` | old I/O API | use `std.Io.File.stdin()` |
| `main has wrong signature` / entry point errors | `main(init: std.process.Init)` | use the 0.16 signature |
| `addExecutable requires .root_module` | old API | use `b.createModule` |
| `expected ';'` in loop | `while (c) : (c2)` comma misplaced | review `while` syntax |
| allocator needed for `allocPrint` | `std.fmt.allocPrint` | use `std.fmt.bufPrint` into a stack buffer, or a writer |
