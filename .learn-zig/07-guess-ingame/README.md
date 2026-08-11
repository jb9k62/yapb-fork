# Lesson 07: `!guess` in-game — the whole game, finally

This is why you wrote `!guess` on the terminal in Lesson 02: **the game logic is byte-for-byte reusable.** You did the interesting design work in a safe sandbox; today you change only the I/O layer — from `std.Io` to the engine's chat.

## Learning Objectives

- Reuse your Lesson 02 `react(secret, guess)` unchanged
- Replace "read guess from stdin" with "parse `!guess N` from chat"
- Replace "print to stdout" with `pfnClientPrintf` (`.chat` / `.center`)
- Keep the secret in a Zig global (explicit, just like `engfuncs`)
- Reset the game with a command

## The mapping (this is the whole lesson)

| Lesson 02 (CLI) | Lesson 07 (in-game) |
|---|---|
| `std.Random` pick secret | same — but store in a `var` global so it survives across chat events |
| `while(input.takeDelimiter('\n'))` | every `pfnClientCommand` with `say !guess ...` is one guess |
| `std.fmt.parseInt(u8, line, 10)` | parse the `!guess <N>` argument |
| `std.debug.print` / `std.Io` | `pfnClientPrintf(ent, .chat, ...)` |
| "you won" → program exits | "you won" → `.center` splash + reset secret |

## Design the state

A global secret that lives between chat events:

```zig
var secret: u8 = 0;
var have_secret = false;

fn new_secret() void {
    var prng = std.Random.DefaultPrng.init(@intCast(std.time.nanoTimestamp()));
    secret = prng.random().intRangeAtMost(u8, 1, 100);
    have_secret = true;
}
```

(`std.Random` works fine inside a plugin — it's pure CPU math, no OS services.)

## Exercise

Extend your Lesson 06 hook with a `!guess` branch:

1. When the `!guess` arg is present with no number → send usage: `"!guess <1-100>"`.
2. Parse the number with `parseInt(u8, n, 10)`; on parse failure reply `"That's not a number!"`.
3. If `!have_secret`, `new_secret()` first.
4. Use your **Lesson 02 `react`** to get the verdict; reply `.chat`.
5. On win → `.center` splash (big text) + `new_secret()` for a fresh round.
6. Support `!guess new` to reset manually.

**Do not rewrite `react`.** Copy it from your Lesson 02 solution verbatim. If you're tempted to change it, stop — that instinct is the whole lesson.

## Solution

`solution/main.zig` — the hook + game state, ready to merge into your bootstrap.

## Verification

Build/deploy as before. In-game:

- `say !guess` → usage text
- `say !guess abc` → "not a number"
- `say !guess 50` → Cold/Hot/Correct, with a `.center` splash on the win
- `say !guess new` → fresh secret

Watch for two classic bugs:
- **Off-by-one / logic drift**: your `react` should be identical to Lesson 02. Run both side-by-side if unsure.
- **Buffer lifetime**: your `msg` buffer must live long enough for `pfnClientPrintf` to read it — keep it in the same scope as the call (stack buffer is fine within one hook invocation).

## What's Next

Lesson 08 — the truly satisfying one: `!whereami` teleport. You'll touch entities and `pfnSetOrigin`, the last piece of "the game is data + engine."

**[Next: Lesson 08 — `!whereami` teleporter](../08-whereami/README.md)**
