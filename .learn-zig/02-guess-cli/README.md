# Lesson 02: `!guess` CLI — the game in pure Zig

The warm-up act for the in-game version. You'll write the **entire game** as a terminal program: a secret number, a loop reading guesses, "warmer/colder", wins and quits. **All the game logic you write here gets copied almost verbatim into the in-game plugin in Lesson 07** — only the I/O layer changes. This is the "write it twice" payoff.

## Learning Objectives

- Loop reading guesses until a win or quit
- `std.Random.DefaultPrng` + `intRangeAtMost`
- `std.fmt.bufPrint` to build a reply string
- Parse text into an integer (`std.fmt.parseInt`)
- Handle bad input without crashing (error union + `catch`)

## Theory quick-hits (open the API sheet too)

**Random with an inclusive range** (API sheet "Random numbers"):
```zig
var prng = std.Random.DefaultPrng.init(@intCast(std.time.nanoTimestamp()));
const rand = prng.random();
const secret = rand.intRangeAtMost(u8, 1, 100);  // 1..=100
```

**Parse a string to an int** — returns `!u8` (error if not a number):
```zig
const n = std.fmt.parseInt(u8, guess_text, 10) catch { print("not a number!\n"); continue; };
```

**Build a reply into a fixed buffer** (so we don't need an allocator):
```zig
var msg: [64]u8 = undefined;
const reply = std.fmt.bufPrint(&msg, "You guessed {d}. Too {s}!", .{ n, "low" }) catch "?";
```
`bufPrint` returns `![]u8` — a slice into your buffer. Use `{s}` for string arguments.

**`continue` / `break`** work like Java/C#/TS. The loop shape is `while`:
```zig
while (true) {
    // read a guess, decide, maybe `break` on win/quit
}
```

## Code references (YaPB talk)

The "warmer/colder" message is the same idea as YaPB's chat responses. When the game starts here, the secret is "a number 1–100"; in Lesson 07 the same secret lives server-side and players guess via chat. Notice how **the logic (compare → reply) never depends on where input comes from** — that's the lesson.

## Exercise — write it all yourself

**File:** `src/main.zig` (we'll add a `build.zig` so you can `zig build run`)

Game rules:
1. Pick a secret integer in `1..=100`.
2. Print `"Guess 1..100 (or 'q' to quit): "`.
3. Read a line. If it's `q`, print `"Bye"` and exit.
4. Parse it as `u8`. On failure, print `"Not a number!"` and continue.
5. Compare: exact → `"Correct!"` + `break`; diff ≤ 5 → `"Hot!"`; else → `"Cold."`
6. Keep a guess counter; show it: `"[3] guess 42: Hot!"`
7. On a correct guess: `"You got it in N guesses! Secret was {d}."`

Also write a `build.zig` (executable) so `zig build run` works. Copy the executable template from `reference/zig-0_16-api.md` ("Build system").

## Hints (only if stuck)

- `const init = ...; const io = init.io;` then reuse the read/write pattern from Lesson 01.
- Read the line, then `if (std.mem.eql(u8, line, "q")) break;` to quit.
- For counter/reply formatting, `bufPrint` with `{d}`.
- Use a `while (true)` loop with `break`.

## Solution

`solution/main.zig` and `solution/build.zig` — open only after attempting.

## Verification

```bash
zig build run        # or: zig run src/main.zig
# Guess 1..100 (or 'q' to quit): 50
# [1] guess 50: Cold.
# Guess 1..100 (or 'q' to quit): 20
# [2] guess 20: Hot!
# ...
# You got it in N guesses! Secret was 23.
```

Also test the boring inputs: `q` quits cleanly; `abc` prints "Not a number!" and keeps going.

## Extra credit (do this — it preps Lesson 07)

Put the whole game in a function `play(io)` and keep the "random secret + compare" as its own function `react(secret, guess) []const u8`. In Lesson 07 that same function becomes the heart of the plugin, and you'll barely change its body.

## What's Next

Lesson 03 — the Zig↔C bridge: call a real C function (our own `helper.c`) from Zig via `@cImport`. This is where Zig starts touching the systems world.

**[Next: Lesson 03 — Zig↔C bridge](../03-cimport/README.md)**
