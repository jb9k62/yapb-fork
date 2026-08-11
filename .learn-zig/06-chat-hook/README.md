# Lesson 06: Chat hook — first in-game output

The payoff lesson. Your plugin now *loads*; this is the one that makes it **talk in-game**. You'll intercept chat, detect a `!` command, and send a message back to the player who typed it.

## Learning Objectives

- Read the chat command: `pfnCmd_Argv`, `pfnCmd_Args`, `pfnCmd_Argc`
- Send a message to a player: `pfnClientPrintf(ent, print_chat, msg)`
- Use `return_meta(.supercede)` vs `.ignored` deliberately
- Build a reply string with `std.fmt.bufPrint`

## Theory: what a "say" looks like to us

A player types `say hello world`. The engine routes the whole thing into our hook `pfnClientCommand(ent)`. The *command* and its *arguments* are read via the engine's command-parsing helpers:

| Field | Value for `say hello world` | Notes |
|---|---|---|
| `pfnCmd_Argv(0)` | `"say"` | the command word |
| `pfnCmd_Argv(1)` | `"hello"` | first argument |
| `pfnCmd_Args()` | `"hello world"` | the raw arg string (all args, no command) |
| `pfnCmd_Argc()` | `3` | number of tokens |

So to catch `/`-style commands players type as `say !guess 42`, you check `pfnCmd_Argv(0)` == `"say"`, then look at the rest for `!...`.

Reply to one player:

```zig
if (engfuncs) |e| e.pfnClientPrintf(ent, .chat, "Hi! Welcome to csguess.");
```

`PRINT_TYPE` is `console`, `center`, `chat` — see `enginefuncs.zig`. `.chat` puts it in that player's chat line; `.center` splashes it in the middle of *their* screen (great for wins, Lesson 07).

**Deciding what happens to the player's original text:**
```zig
return_meta(.supercede); // block their 'say' from broadcasting (consume it)
return_meta(.ignored);   // let their 'say' through to everyone (just listen)
```

## Code references

- `src/linkage.cpp:320` — YaPB reads the same way: `engfuncs.pfnCmd_Argv(0)` + `.pfnCmd_Argv(1)`.
- `src/linkage.cpp:803` — YaPB prints to a player with `engfuncs.pfnClientPrintf(ent, printType, message)`.
- `enginefuncs.zig` — the `pfnCmd_*` and `pfnClientPrintf` signatures you transcribed.

## Exercise

Start from your finished Lesson 05 bootstrap. In `our_client_command` (rename it something meaningful like `on_client_command`):

1. Read the command: `const cmd = std.mem.span(e.pfnCmd_Argv(0));` and the whole args: `const args = std.mem.span(e.pfnCmd_Args());`
2. If `cmd` is not `"say"`, `return_meta(.ignored)` and return (we only care about chat).
3. If `args` starts with `!greet`, print `"Hey, you typed a command!"` to the player via `.chat`.
4. If it starts with `!who`, reply with the argument count and the raw args, e.g. `"cmd: {d} args: {s}"` using `bufPrint`.
5. `return_meta(.supercede)` for the `!` commands (so the `!greet ...` text itself doesn't flood chat), otherwise `.ignored`.

Tiny helper for working with C strings → Zig slices:

```zig
fn cstr(s: [*:0]const u8) []const u8 {
    return std.mem.sliceTo(s, 0); // up to and including first NUL
}
```

## Hints

- `pfnCmd_Argv`/`pfnCmd_Args` return `[*:0]const u8`; use `std.mem.sliceTo(ptr, 0)` to get a bounded slice.
- Prefix check: `std.mem.startsWith(u8, args, "!greet")`.
- One player at a time → `return_meta(.ignored)` for normal chat so teammates still see it.

## Solution

`solution/main.zig` — a drop-in replacement for the Lesson 05 scaffold's hook section.

## Verification

Build + deploy (as in Lesson 05), then in-game:

- `/ say !greet` → **you** see `Hey, you typed a command!`
- `/ say hello` → nothing (chat still broadcasts normally)
- `/ say !who` → you see the arg count + raw args

If the `!` message echoes *and* your reply shows, that's fine — check you used `.supercede` for `!` commands (or `.ignored` if you *want* both).

## What's Next

Lesson 07 — wire the `!guess` game from Lesson 02 into this exact hook. Same logic, new I/O.

**[Next: Lesson 07 — `!guess` in-game](../07-guess-ingame/README.md)**
