# The GoldSrc / MetaMod Plugin ABI

How "extension" plugins for Counter-Strike 1.6 actually work — grounded in YaPB's `src/linkage.cpp` and `ext/linkage/linkage/`.

> This is the ONLY genuinely hard part of the whole curriculum, and it's identical no matter what your plugin does. Read this to *understand* it; Step 5 gives you the working scaffold.

## What "a plugin" even is

A `.so` (Linux) or `.dll` (Windows) shared library that the engine **dlopens** into its own process. It exports a fixed set of C functions MetaMod calls, and receives, as arguments:

- **function tables** — structs full of function pointers that MetaMod/the engine use to call *your* code
- **the engine functions** — a big table of pointers *you* can call to make the engine do things (print chat, move entities, etc.)

Everything is just memory: two structs pointing at functions, plus a global that says "did the last hook get handled?"

## The chain

```
Engine (hl.so)
   │  dlopen
   ▼
MetaMod (metamod.so)          ← routes calls to loaded plugins
   │  dlopen
   ├── YaPB (yapb.so)          ← your bot
   └── csguess (libcsguess.so) ← OUR Zig plugin (this curriculum)
           │  forwards most calls
           ▼
Counter-Strike (cs.so)
```

YaPB itself is the bot; we write a *second, tiny* plugin that MetaMod loads alongside it.

## The global state trick

GoldSrc uses a classic C API pattern: a single mutable global function-pointer table.

```cpp
// src/linkage.cpp
enginefuncs_t engfuncs {};     // the engine's function table, copied in by the engine
meta_globals_t *gpMetaGlobals; // "did the last hook get consumed?" state
```

The engine *passes* `engfuncs` (its function table) to your DLL, you copy it into your own global, then call through it:

```cpp
engfuncs.pfnClientCommand(ent, "say hi");     // make the engine do something
engfuncs.pfnServerPrint("hello");             // print to server console
```

**Why this matters for Zig:** Zig has no magic globals. You declare the global yourself (`var engfuncs: ?*enginefuncs_t = null;`) and copy the pointer in at attach time. That's a feature, not a bug — it makes you aware there IS a global. (Step 4–5.)

## The four functions MetaMod exports

A MetaMod plugin must `export` these four C functions (see `src/linkage.cpp`):

| Export | What MetaMod wants |
|---|---|
| `Meta_Init()` | run first; lets you detect you're under MetaMod |
| `Meta_Query(char *ifvers, plugin_info_t **info, mutil_funcs_t *util)` | hand back a filled `plugin_info_t` + verify interface version |
| `Meta_Attach(PLUG_LOADTIME, metamod_funcs_t *tbl, meta_globals_t *mg, gamedll_funcs_t *gd)` | copy your function table into `tbl`, save the engine tables, install hooks |
| `Meta_Detach(PLUG_LOADTIME, PL_UNLOAD_REASON)` | clean up, uninstall hooks |

Exported in C as:

```cpp
CR_EXPORT int Meta_Query(char *ifvers, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs);
CR_EXPORT int Meta_Attach(PLUG_LOADTIME now, metamod_funcs_t *functionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs);
CR_EXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason);
CR_EXPORT void Meta_Init();
```

`CR_EXPORT` = `extern "C"` visibility; in Zig that's `export fn ... callconv(.c)`.

## The function tables (the ceremony core)

There are three interlocking tables. See `ext/linkage/linkage/metamod.h`.

1. **`metamod_funcs_t`** — what *your plugin* fills and hands MetaMod in `Meta_Attach`. It's a list of `GetEntityAPI` / `GetEngineFunctions` entry points (functions *you* wrote) that MetaMod calls to obtain the rest:

```c
struct metamod_funcs_t {
    GETENTITYAPI_FN pfnGetEntityAPI;        // "give me your DLL's API table"
    GETENTITYAPI_FN pfnGetEntityAPI_Post;   // same, called *after* the real game
    ...
    GET_ENGINE_FUNCTIONS_FN pfnGetEngineFunctions;
    ...
};
```

2. **`gamefuncs_t`** (the "dllapi" table) — the game engine's list of *callbacks into the game* (`pfnClientCommand`, `pfnSpawn`, `pfnKeyValue`, ...). As a plugin you create your **own** `gamefuncs_t`, install your version of a callback, and (optionally) chain to the real game's via `gamedll_funcs_t.dllapi_table`. **Our chat hook lives here** as `pfnClientCommand`.

3. **`enginefuncs_t`** — the *engine's* service table (you call `engfuncs.pfnClientPrintf(...)` to send chat). This is the big one we hand-transcribe in Step 4.

## How OUR chat hook works

Players type `say hello`. The engine routes it into the game DLL's `pfnClientCommand(ent)`. Because we're hooked, **our** `pfnClientCommand` runs first:

1. We check the argument: the *command* part (`say`) via `engfuncs.pfnCmd_Argv(0)`, and the *text* via `pfnCmd_Args()` / `pfnCmd_Argv(n)`.
2. If it starts with `!guess`, we run our game logic.
3. We reply via `engfuncs.pfnClientPrintf(ent, print_chat, "warmer")` — this prints a message to that one player's chat.
4. We `RETURN_META(MRES_SUPERCEDE)` to stop the "say" from broadcasting normally (optional — we can also let it through).

YaPB does the same interception at `src/linkage.cpp:291` and classifies the command:

```cpp
table->pfnClientCommand = [] (edict_t *ent) CR_FORCE_STACK_ALIGN { ... };
```

## What we call through the engine (our whole API surface)

From `ext/linkage/linkage/goldsrc.h` (`enginefuncs_t`, lines 1047–1161):

| Field | Signature | What it does |
|---|---|---|
| `pfnClientPrintf` | `(edict_t*, PRINT_TYPE, const char*)` | print chat/center text to one player |
| `pfnCmd_Argv` | `(int) -> const char*` | get a chat/console argument by index |
| `pfnCmd_Args` | `() -> const char*` | get the whole argument string |
| `pfnCmd_Argc` | `() -> int` | count of arguments |
| `pfnSetOrigin` | `(edict_t*, const float*)` | teleport an entity (`!whereami`, Step 8) |
| `pfnPEntityOfEntIndex` | `(int) -> edict_t*` | get a player's entity by ID |
| `pfnTime` | `() -> float` | server clock (for state timeouts) |
| `pfnGetPlayerUserId` | `(edict_t*) -> int` | unique player id |

`PRINT_TYPE` (goldsrc.h:467): `print_console=0`, `print_center=1`, `print_chat=2`.

Because all of these are **function pointers in one table**, the table is laid out as a single array of pointers — that's why we can transcribe it to a Zig `extern struct` where "every member is a function pointer" and the offsets are automatically correct (Step 4).

## The "you only need the prefix" insight

An `extern struct` doesn't need every member. Since layout is determined by order, you only need the **leading members up to the highest one you call**. Everything after can be omitted. `@offsetOf(enginefuncs_t, "pfnClientPrintf")` will prove the offset matches the C header. This is exactly why Step 4 is worth doing by hand once.

## What to read in the codebase

- `src/linkage.cpp` — `Meta_Query`/`Meta_Attach`/`Meta_Detach`/`Meta_Init` (lines ~955–1010), the `pfnClientCommand` hook (~291).
- `ext/linkage/linkage/metamod.h` — the three table structs + `RETURN_META` macros.
- `ext/linkage/linkage/goldsrc.h` — `enginefuncs_t` (1047), `PRINT_TYPE` (467), `edict_t`/`entvars_t` (900/1044).
