# Lesson 01: Hello, Zig — core syntax on the terminal

First Zig program that actually *does* something: read a line, do something per char, print a result. No engine, no C — pure Zig muscle memory.

## Learning Objectives

- Use the 0.16 entry point `main(init: std.process.Init) !void`
- Print with `std.debug.print` and the 0.16 `std.Io` writer
- Read a line from stdin with `readerStreaming` + `takeDelimiter`
- Write your first slices, `for`, optional, and `defer`

## Theory quick-hits

Open `reference/zig-cheatsheet.md` for the full translations. The four things you'll type in this lesson:

**1. Slices are the string.**
```zig
const name: []const u8 = "jordan";
const first2 = name[0..2];   // "jo"
```

**2. `for` with an index.**
```zig
for ("hello", 0..) |ch, i| { std.debug.print("i={d} ch={c}\n", .{ i, ch }); }
```
`{c}` prints a single byte as a character; `{d}` prints an integer.

**3. Optionals captured by `while`/`if`.**
```zig
while (try stream.takeDelimiter('\n')) |line| {
    // `line` is non-null here
}
```
`try` propagates the error; `|line|` unwraps the optional. Lines 54-60 of the API sheet.

**4. `defer` runs at scope exit.**
```zig
var prng = std.Random.DefaultPrng.init(1);
defer std.debug.print("bye\n", .{});
_ = prng;
```
Runs even on early returns. (You'll use `defer` more seriously later, but plant the habit now.)

## Code references (YaPB)

Curiosity: peek how YaPB logs things — everything funnels through the engine's `pfnServerPrint`, which we'll call from Zig in Lesson 06. For now, just know `std.debug.print` is your *local* stand-in for that.

## Exercise — write it yourself

**File:** `src/main.zig` (in a fresh `01-hello/` you create, or directly via `zig init`)

Write a program that:
1. Prints `"What's your name?"`
2. Reads one line from stdin
3. Strips any trailing `\r` (Windows newline) — hint `.takeDelimiter('\r')` no... just slice off the `\n` via `takeDelimiter('\n')` and ignore the trailing line
4. Prints `"hello <name>, welcome to zig!"` to stdout with `std.Io`
5. Then prints each byte of the name with its index, e.g. `0:j 1:o ...`

Use **all four** concepts above. Don't open `solution/` until you're stuck past 10 minutes.

You can iterate quickly without build.zig:

```bash
zig run src/main.zig
```

## Hints (only if stuck)

- `const io = init.io;` gets you the I/O owner you hand to `writerStreaming`/`readerStreaming`.
- Writer: `var w = std.Io.File.stdout().writerStreaming(io, &buf);` then `var out = &w.interface;` then `try out.print(...)` and `try w.flush();`.
- Reader: same shape but `stdin()` and `readerStreaming`, giving `var input = &r.interface;` then `while (try input.takeDelimiter('\n')) |line|`.
- You need two separate buffers (one for writing, one for reading) — a slice is borrowed, not owned.

## Solution

`solution/hello.zig` — open only after you've attempted it.

## Verification

```bash
zig run src/main.zig
# What's your name?
# <type:  jordan>
# hello jordan, welcome to zig!
# 0:j 1:o 2:r 3:d 4:a 5:n
```

The per-char loop proves you can iterate a string with an index — the same skill you'll use to parse `say !guess 42` later.

## What's Next

Lesson 02 — the actual `!guess` game as a terminal CLI, where loops, random and string building come together.

**[Next: Lesson 02 — `!guess` CLI](../02-guess-cli/README.md)**
