# Lesson 04: C ABI by hand — `extern struct`, `@offsetOf`, `export`

Now the real ABI skill: **reproduce a C struct's layout as a Zig `extern struct`** and **export a C-callable function** back out. You'll do both on real engine types. No engine, no MetaMod yet — just you, a struct, and the compiler proving you understand memory.

## Learning Objectives

- Transcribe a C `struct` into a Zig `extern struct`
- Verify layout with `@offsetOf` and `@sizeOf` at comptime
- `export fn ... callconv(.c)` to expose a symbol to C
- Appreciate the "you only need the prefix" insight

## Theory

**`extern struct`** is a struct with guaranteed C layout — fields in order, natural alignment, no reordering. That's the contract for interoperating with C.

**`@offsetOf(T, "field")`** is a comptime constant giving a field's byte offset. **`@sizeOf(T)`** gives total size.

```zig
const P = extern struct { a: u8, b: f32 };
// a at offset 0, b at offset 4 (f32 aligns to 4), size 8
comptime { std.debug.assert(@offsetOf(P, "b") == 4); }
```

**Why function-pointer tables are trivial to transcribe:** every member is a pointer, pointers are one machine word, so the struct is literally an array of pointers. There is no padding or alignment subtlety *between* function pointers. That's the entire reason the engine table (Step 5) is safe:

```zig
const enginefuncs_t = extern struct {
    pfnPrecacheModel: *const fn ([*:0]const u8) callconv(.c) c_int,
    pfnSetOrigin:     *const fn (?*edict_t, [*]const f32) callconv(.c) void,
    // ...
};
```

**The prefix trick:** you don't need *all* members. Only the leading ones up to the highest you call. `enginefuncs.zig` in this curriculum does exactly that (drops everything after `pfnGetPlayerUserId`).

**`export fn`** makes a symbol visible to C and to `dlopen`/`nm`:
```zig
export fn game_started() c_int { return 1234; }
```
Add `callconv(.c)` when the signature must match C precisely. This is literally what the plugin's `Meta_*` functions are.

## Code references

- `ext/linkage/linkage/goldsrc.h:1047` — the real `enginefuncs_t` (compare your transcription against it).
- `ext/linkage/linkage/metamod.h` — the small metamod tables you'll transcribe in Lesson 05.

## Exercise

You have the engine table already transcribed in `reference/enginefuncs.zig`. Your job:

1. **Copy** `reference/enginefuncs.zig` into your project as `src/enginefuncs.zig` (keep the `PRINT_TYPE` and `enginefuncs_t`).
2. Add comptime assertions in `src/main.zig` (`@import("enginefuncs.zig")`) that print:
   - `@offsetOf(enginefuncs_t, "pfnSetOrigin")`
   - `@offsetOf(enginefuncs_t, "pfnClientPrintf")`
   - `@sizeOf(enginefuncs_t)`
3. Add an `export fn game_started() c_int` returning `1234`.
4. Build a **shared library** (`build.zig` with `.linkage = .dynamic`), then confirm the symbol:
   ```bash
   nm -D zig-out/lib/libengine.so | grep game_started
   ```
   (You'll see `T game_started` — available for C/dlopen to call.)

Don't skip step 2: asserting the offsets at comptime is what makes a transcription you *typed* trustworthy.

## Hints

- Import: `const eng = @import("enginefuncs.zig");`
- Print comptime values with `std.debug.print("{d}\n", .{@offsetOf(eng.enginefuncs_t, "pfnSetOrigin")});`
- Shared-library build template is in `reference/zig-0_16-api.md`.

## Verification

`zig build` succeeds (comptime asserts pass), and:

```bash
zig build -Dtarget=x86-linux-gnu   # confirm it also cross-compiles to 32-bit
nm -D zig-out/lib/libengine.so | grep -E 'game_started|pfn|PRINT'
```

The offsets print as large numbers (hundreds of bytes) — that's the *real* distance into the C table, and it's exactly the offset a C program compiled against the header computes. **That's the proof it's correct.**

## What's Next

Lesson 05 — the MetaMod skeleton. Now that structs/symbols make sense, you meet the four `Meta_*` exports and the three function tables. I scaffold the boilerplate; you re-type and run it.

**[Next: Lesson 05 — MetaMod skeleton](../05-metamod-skeleton/README.md)**
