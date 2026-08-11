# YaPB Zig Curriculum — Learn Zig by modding CS 1.6

A hands-on path for TypeScript / C# / Java / Python developers who want to learn **C and Zig** through the thing we both love: **Counter-Strike 1.6 modding**.

You will build one small, real, in-game extension from scratch over 9 lessons — a chat-driven **`!guess` number-guessing game** — plus a `!whereami` teleporter as a bonus. Along the way you learn Zig itself, the C ABI, and how Half-Life "extension" plugins actually work (the same mechanism YaPB uses).

> **Version pin:** This curriculum targets **Zig 0.16.0** (current stable). Zig's stdlib changes aggressively between versions, so **install exactly 0.16.0** and keep `reference/zig-0_16-api.md` at hand. If a snippet won't compile, check the version first.

---

## The philosophy

- **You type the code.** I only hand you theory primers and the genuinely tedious boilerplate (the MetaMod bootstrap). You type everything else to build muscle memory.
- **Every lesson runs something visible.** No abstract steps — each ends with "you ran it and saw it work."
- **You write `!guess` twice.** First as a pure Zig terminal CLI (no engine, no ABI — learn Zig syntax safely), then wired into the game. That separation keeps "learning ABI" and "learning Zig" from colliding.
- **The dead ceremony is isolated.** The MetaMod bootstrap (Step 5) is the hardest part and it's identical no matter what your plugin does, so I provide it. After that, everything else is just Zig you already know.

---

## Lessons

| # | Lesson | Builds | You write | I scaffold | Concepts |
|---|--------|--------|-----------|------------|----------|
| 00 | [Setup](00-setup/README.md) | toolchain | everything | — | install Zig 0.16, project layout, cross-compile idea |
| 01 | [Hello & core Zig](01-hello/README.md) | terminal print | all | build.zig | `std.Io`, `fn`, slices, optionals, errors, `defer` |
| 02 | [`!guess` CLI](02-guess-cli/README.md) | terminal game | **all game logic** | build.zig | loops, `std.Random`, string formatting, "warmer/colder" |
| 03 | [Zig↔C bridge](03-cimport/README.md) | Zig calls a `.c` fn | call site + build | `helper.c` | `@cImport`, `extern`, `callconv(.c)`, linking C |
| 04 | [C ABI by hand](04-extern-abi/README.md) | translate a C struct | the struct + export | test harness | `extern struct`, `@offsetOf`, `export fn`, memory layout |
| 05 | [MetaMod skeleton](05-metamod-skeleton/README.md) | plugin loads | re-type/run each piece | **full bootstrap** | plugin lifecycle, func tables, globals in Zig |
| 06 | [Chat hook: first in-game output](06-chat-hook/README.md) | reply to chat | the hook + parse | hook wiring | `pfnClientCommand`, `pfnCmd_Argv`, `pfnClientPrintf` |
| 07 | [`!guess` in-game](07-guess-ingame/README.md) | full game | **the whole game** | — | all of it, `print_center`, per-player state |
| 08 | [`!whereami` teleporter](08-whereami/README.md) | teleport players | TP + entity math | entity lookup | `edict_t`, `pfnSetOrigin`, entity indexing |

**Time:** lessons 0–3 are 30–60 min each; 4 is ~1 hr; 5 is the long one (~1–2 hr); 6–8 snap together fast.

---

## Reference files

Read these alongside the lessons (not before — you'll appreciate them more in context):

- [`reference/zig-0_16-api.md`](reference/zig-0_16-api.md) — pinned 0.16.0 API cheatsheet (entry point, `std.Io`, `std.Random`, `build.zig`).
- [`reference/zig-cheatsheet.md`](reference/zig-cheatsheet.md) — TypeScript / C# / Java / Python → Zig translations + mental-model shifts.
- [`reference/goldsrc-plugin-abi.md`](reference/goldsrc-plugin-abi.md) — how the GoldSrc/MetaMod plugin ABI works, grounded in YaPB's `src/linkage.cpp`.
- [`reference/enginefuncs.zig`](reference/enginefuncs.zig) — the hand transcription of `enginefuncs_t` (the engine function table) we use in Steps 4–8.

The C++ curriculum (`.learn/`) is a great second pass later — it teaches the *same* engine concepts from the C++ side, so you can read YaPB's real bot code.

---

## Recommended workflow per lesson

```bash
# 1. Read the Theory section (with the cheatsheet open)
# 2. Explore the Code References in the actual repo (YaPB src/)
# 3. Type the exercise yourself — do NOT open the solution yet
# 4. Only if stuck >10 min, peek at the provided solution
# 5. Follow Verification and confirm it runs
```

## The honest warning, in one line

Step 5's MetaMod bootstrap is real ceremony. Don't fight it — my scaffold does it; your job is to *understand and re-type* it, then all the fun (Steps 6–8) pays off instantly.

## Next step

Start with **Lesson 00: Setup** — install Zig 0.16.0 and get `zig version` green.

**[→ Lesson 00: Setup](00-setup/README.md)**
