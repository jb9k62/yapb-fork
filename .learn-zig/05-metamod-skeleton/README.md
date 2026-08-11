# Lesson 05: The MetaMod skeleton — the ceremony you'll re-type

This is the **single hardest part** of the whole curriculum: the bootstrap that lets a `.so` load into CS 1.6 under MetaMod at all. It is identical no matter *what* your plugin does, so **I scaffold it fully** and your job is to read it carefully, re-type it, and get it loading. Everything fun after this is just Zig you already know.

> Why are we using MetaMod? It's the standard way to add an extension alongside YaPB, and it's exactly what YaPB itself is (`src/linkage.cpp`). After this lesson you'll understand every GoldSrc/Half-Life extension you ever see.

## Learning Objectives

- Understand the 4 exported `Meta_*` functions and when MetaMod calls them
- See how Zig represents C function tables as `extern struct`s in action
- See the "global state in Zig is explicit" pattern (`var engfuncs: ?*enginefuncs_t`)
- Load the plugin and confirm MetaMod sees it (`meta list`)
- Read `reference/goldsrc-plugin-abi.md` end to end first

## Read this first (10 min)

`reference/goldsrc-plugin-abi.md` — it explains the three interlocking tables and the MetaMod lifecycle. The scaffold below is a direct transcription of that doc, backed by YaPB's `src/linkage.cpp`. Have both open while re-typing.

## What the four exports do

| Export | When MetaMod calls it | What we do |
|---|---|---|
| `Meta_Init` | very first | (optional) note we're live |
| `Meta_Query(ifvers, **info, *util)` | before attach | save `util`, hand back `Plugin_info`, version-check |
| `Meta_Attach(now, *ftable, *globals, *gamedll)` | loading | copy our `metamod_funcs_t`, save `gpMetaGlobals`, fetch engine table |
| `Meta_Detach(now, reason)` | unloading | cleanup |

Plus two functions MetaMod calls *through* our `metamod_funcs_t` to install hooks:

| Table entry | When | What we do |
|---|---|---|
| `GetEntityAPI(table, ver)` | attach/changelevel | zero `table`, install `table->pfnClientCommand = our hook` |
| `GetEngineFunctions(table, ver)` | attach | no-op (we don't intercept engine calls) |

## Code references (YaPB, the authoritative source)

- `src/linkage.cpp:955` — `Meta_Query`
- `src/linkage.cpp:990` — `Meta_Attach` (how `metamod_funcs_t` is built)
- `src/linkage.cpp:1042` — `Meta_Init`
- `src/linkage.cpp:80` — `GetEntityAPI` (the `bzero` + install-hooks pattern)
- `src/linkage.cpp:578` — `GetEngineFunctions`
- `ext/linkage/linkage/metamod.h` — the table structs & `RETURN_META`

## The scaffold (I provide — re-type it)

Files: `scaffold/main.zig`, `scaffold/enginefuncs.zig` (copy from `reference/`), `scaffold/build.zig`.

When you re-type, notice **how little is "plugin logic"** — almost everything is plumbing a C ABI. That's the point: once it's done, it's done forever.

Key global-state pattern (compare to YaPB's `enginefuncs engfuncs` global):

```zig
// global engine table, set once at attach, read by every hook
var engfuncs: ?*enginefuncs_t = null;   // placement in real code: container-level
var gp_globals: ?*meta_globals_t = null;
var gp_mutil: ?*mutil_funcs_t = null;
```

Fetching the real engine table at attach (the one non-obvious line):

```zig
// metamod gives plugins the merged engine/gamedll tables via pfnGetHookTables
var eng_ptr: ?*enginefuncs_t = null;
var dll_ptr: ?*gamefuncs_t = null;
var newdll_ptr: ?*newgamefuncs_t = null;
_ = gp_mutil.?.pfnGetHookTables(&plugin_info, &eng_ptr, &dll_ptr, &newdll_ptr);
engfuncs = eng_ptr;
```

> If your metamod build differs from YaPB's (rare), reconcile against `ext/linkage/linkage/metamod.h` and `src/linkage.cpp` — the Zig here is a faithful transcription of the working C++.

## Install & load it

This lesson assumes MetaMod is set up (YaPB's `00-setup` documents CS 1.6 + a game; for MetaMod, add to `cstrike/addons/metamod/plugins.ini`):

```ini
linux addons/csguess/libcsguess.so
```

Build 32-bit:

```bash
cd scaffold
zig build -Dtarget=x86-linux-gnu
ls zig-out/lib/        # -> libcsguess.so
cp zig-out/lib/libcsguess.so /path/to/cstrike/addons/csguess/
```

Launch CS 1.6, open console, and run:

```
meta list
```

You should see a line with **csguess** and status `RUN`. If it's not there, check the server console; a mismatch shows as a meta-interface version error (the `Meta_Query` check exists for exactly this).

## Exercise (your part)

1. **Copy `reference/enginefuncs.zig` into `scaffold/`** (the engine table is the shared foundation).
2. Re-type `main.zig` and `build.zig` from the scaffold, **commenting every block in your own words** as you go — that's how you make the ceremony yours.
3. Build, deploy, and get `meta list` to show it `RUN`.
4. Add a line logging a marker at attach so you *know* it attached:
   ```zig
   if (engfuncs) |e| e.pfnServerPrint("csguess attached!\n");
   ```

## Verification

- [ ] `zig build -Dtarget=x86-linux-gnu` succeeds → `libcsguess.so` (32-bit)
- [ ] `file zig-out/lib/libcsguess.so` says `ELF 32-bit`
- [ ] in-game `meta list` shows `csguess ... RUN`
- [ ] server console prints `csguess attached!` on load

This is the only lesson where "did it work?" isn't a game feature — it's *a plugin that loads without crashing*. Celebrate that; it's the hard 80% done.

## What's Next

Lesson 06 — now that it loads, make it *do* something in-game: intercept chat and reply. The first visible payoff.

**[Next: Lesson 06 — Chat hook: first in-game output](../06-chat-hook/README.md)**
