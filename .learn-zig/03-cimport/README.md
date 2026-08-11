# Lesson 03: Zig ↔ C — calling a `.c` file from Zig

Zig's headline feature is that it *is* a C toolchain: it compiles C, links C, and imports C headers. In this lesson you'll write a tiny C helper and **call it from Zig** via `@cImport`. This is the on-ramp to the engine plugin — but note the twist at the bottom about why we *won't* `@cImport` the real engine headers.

## Learning Objectives

- Write a small `.c` + `.h`
- `@cImport` a C header from Zig
- Call a C function and read a C `struct` from Zig
- Link a `.c` source into a Zig build (`addCSourceFile`)
- Understand the honest reason `@cImport` *won't* work on YaPB's headers

## Theory

**`@cImport`** translates a C header into Zig types at compile time:

```zig
const c = @cImport({
    @cInclude("helper.h");
});

// now use it:
const x = c.add_ish(3, 4);          // call the C function
const s = c.PointNS{ .x = 1, .y = 2 }; // construct a C struct
```

**`extern`** is the boundary keyword. Even without `@cImport`, you can declare an existing C function yourself:

```zig
extern fn sqrt(f64) f64; // declares an already-existing symbol, no header needed
```

`@cImport` is just convenience that also imports *types* (structs, enums, typedefs).

**Linking a C file** in `build.zig` (0.16) — the C source joins the same module:

```zig
const exe = b.addExecutable(.{
    .name = "bridge",
    .root_module = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    }),
});
// add our C source to the module:
exe.root_module.addCSourceFile(.{ .file = b.path("src/helper.c"), .flags = &.{} });
```

Now `helper.c` gets compiled and linked, and its symbols are callable.

## Code references

YaPB's whole existence is Zig-style `extern` calls into the engine — but from C++, e.g. `src/linkage.cpp` calls `engfuncs.pfnClientPrintf(...)`. The *concept* (call into a table of C function pointers) is what Lessons 4–5 build on.

## Provided: `src/helper.c` and `src/helper.h`

I've written a tiny C helper for you to call (see `helper.c` in this folder).

`helper.h`:
```c
typedef struct Vec2 { float x; float y; } Vec2;
float vec_len(Vec2 v);
```

## Exercise — write the Zig side yourself

**File:** `src/main.zig` + `build.zig`

1. `@cImport` `helper.h`
2. Call `vec_len` with a `Vec2{ .x = 3, .y = 4 }`, print the result (expect `5`)
3. Construct a second `Vec2` directly and pass it
4. Add a line proving you *see* the C type: `std.debug.print("Vec2 size = {d} bytes\n", .{@sizeOf(c.Vec2)});`

Write `build.zig` that compiles and links both `main.zig` and `helper.c`.

## The honest twist (read this)

You might think: "great, so Lesson 05 will `@cImport` the engine headers!" **It can't.** YaPB's `ext/linkage/linkage/goldsrc.h` and `metamod.h` are **C++** (they use `constexpr`, `cr::Vector`, classes) — and `@cImport` only accepts *C*. So in the plugin we do something more valuable anyway: **hand-transcribe the C struct layouts into Zig `extern struct`s** (Lesson 04). That's the real ABI skill. You just learned the tool that works when C is available; next you learn the skill that works always.

## Verification

```bash
zig build run
# vec_len({3,4}) = 5
# Vec2 size = 8 bytes
```

The `context: 5` result and `8 bytes` (two `f32`s / `@sizeOf`) both confirm the calling convention and layout line up.

## What's Next

Lesson 04 — transcribe a C struct by hand into `extern struct`, verify with `@offsetOf`, and `export` a C-callable function back out. The full ABI handshake, minus the engine.

**[Next: Lesson 04 — C ABI by hand](../04-extern-abi/README.md)**
