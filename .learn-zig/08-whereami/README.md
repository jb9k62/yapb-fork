# Lesson 08: `!go X Y Z` — teleport players (entities in motion)

The last piece. You'll make players *move through the world* by writing to the engine's entity system through `pfnSetOrigin`. This is the gateway to real mods: physics, spawns, item drops, nade spawns — it's all "entities are data, engine is the function table."

## Learning Objectives

- Understand `edict_t` as an opaque entity handle you *pass* (you rarely dereference it)
- Call `pfnSetOrigin(ent, &origin[3])` to place an entity in 3D space
- Parse three floats from chat (`std.fmt.parseFloat`)
- (bonus) Recognize what you'd need to *read* a position (`entvars_t`)

## Theory: the entity, in one line

On the server, *everything* is an `edict_t` — a player, a weapon, a nade, a door. You almost always just **pass it around** by pointer rather than reading its guts. That's why we typed it as `opaque {}` in `enginefuncs.zig` and it worked.

To move an entity you don't touch its fields at all — you call an *engine function*:

```zig
// a point in the world: 3 floats, contiguous
var dest: [3]f32 = .{ 0.0, 0.0, 0.0 }; // map origin (usually sea level / world 0)

// drop the player there
_ = e.pfnSetOrigin(ent, &dest);
```

`&dest` on a `[3]f32` gives `*[3]f32` → coerces to `[*]const f32`, which is exactly the C `const float *rgflOrigin`. The engine copies those 3 floats into the entity's `v.origin` internally. **You never touch the struct; the engine does.**

> **Bonus / stretch (honest note):** To *read* a player's current position (e.g. a `!pos` command or "teleport everyone to spawn"), you must traverse `entvars_t` — a huge struct starting with the entity's mutable state, whose `origin` field lives deep in it. That's another `extern struct` transcription (the same skill as `enginefuncs.zig`, just bigger). We skip it here; `!go` passes coordinates *in*, which avoids the read entirely.

## Code references

- `ext/linkage/linkage/goldsrc.h:1076` — `pfnSetOrigin(edict_t *e, const float *rgflOrigin)`
- `ext/linkage/linkage/goldsrc.h:1120` — `pfnPEntityOfEntIndex(int)` → `edict_t*` (reach *other* entities by id)
- `src/botlib.cpp` & `src/botmove*` — YaPB reads `pent->v.origin` constantly (there it *does* dereference `entvars_t`, because it's C++)
- `enginefuncs.zig` — the `pfnSetOrigin` you transcribed

Our `!go` uses the `ent` already handed to us by the hook (the player who typed). We don't even need `pfnPEntityOfEntIndex` for this — but keep it in mind for `!tp <name>`.

## Exercise

Add two commands to your Lesson 07 hook state:

1. **`!go X Y Z`** — teleport the speaker there.
   - Parse three `f32` from the args: `std.fmt.parseFloat(f32, ...)`.
   - On parse failure → `.chat` `"Usage: !go X Y Z"`.
   - Clamp to a sane box (e.g. `-4096..4096`) so nobody gets thrown into a void.
   - `pfnSetOrigin(ent, &dest)`, then `.chat` `"You moved to X Y Z"`.
2. **`!here`** (the stretch, only if you transcribed `entvars_t`... otherwise replace with a thank-you) — but to keep this lesson achievable, make `!here` teleport you to **map center `(0,0,0)`** instead. That still proves the call works AND gives you a fun "reset me to spawn zone" button:
   - `pfnSetOrigin(ent, &.{ 0, 0, 0 })` — note the `&.{...}` inline anonymous array (a Zig nicety: an anonymous list literal).

## Hints

- Calling with an inline array: `e.pfnSetOrigin(ent, &.{ 0, 0, 0 })` compiles the `[3]f32` on the spot and coerces to the `[*]const f32`.
- For parsing, split the args: `const rest = std.mem.trim(u8, args["!go".len..], " ");` then walk three whitespace-separated tokens (you can reuse `std.mem.tokenizeAny`).
- Keep `return_meta(.supercede)` so `!go ...` doesn't echo its own coords to chat.

## Solution

`solution/main.zig` — the `!go` + `!here` branches.

## Verification

Build/deploy. In-game on any map:

- `say !go 100 200 300` → you teleport; chat confirms `You moved to X Y Z`
- `say !go abc` → usage text
- `say !here` → you snap to world origin `(0,0,0)`
- `say !go 99999 99999 99999` → your clamp kicks in (no void)

If you get thrown into a wall or the void, check your clamp and the axis order (X forward/right, Z up in GoldSrc — the third number is height).

## What's Next — you built a real extension

From `liblist.gam` bootstrapping to chat hooks to the entity API, you've written a real, loading, interactive CS 1.6 extension in Zig. Ideas to grow it:

- `!tp <name>` — resolve another player via `pfnPEntityOfEntIndex` and teleport *them*.
- Multi-round `!guess` with per-player scores (a `std.StringHashMap` keyed by `pfnGetPlayerUserId`).
- Broadcast center messages to *all* players (transcribe `pfnMessageBegin`/`MSG_BROADCAST`).
- Port your favorite YaPB feature to Zig.

And when you're ready for the other side: the companion C++ curriculum in `.learn/` teaches the same engine from the C++ angle so you can read YaPB's actual bot code.

**[→ Back to the curriculum index](../README.md)**
