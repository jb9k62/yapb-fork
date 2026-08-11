# TypeScript / C# / Java / Python → Zig

A translation table for when you're typing Zig and your brain reaches for a familiar construct. Zig is the most "explicit" of these languages — almost everything that's implicit elsewhere is written out.

## Mental model shifts (the big ones first)

| Your toolkit | In Zig | Why it's different |
|---|---|---|
| Garbage collector | **No GC.** You choose (stack, fixed buffer, or allocator). | Zig gives you memory ownership you *must* think about — the whole point of studying systems. |
| `null` / `undefined` / `None` / `Optional` | `?T` — the nullable part is *only* the `?` | Non-optional values are guaranteed non-null at compile time. |
| Exception / try-catch | `!T` error unions + `try`/`catch` + `defer` | Errors are *values*, no hidden stack unwinding machinery. `defer` guarantees cleanup on error paths. |
| Classes | `struct` (plain data) + free functions | No inheritance, no OOP by default. You compose data + behavior explicitly. |
| TypeScript `let x: number` | `var x: i32` or `const x: i32` | `const` = immutable *binding* (like final/readonly), `var` = mutable. Types are sized and signed literally. |
| `interface` / `abstract` | nested `fn` fields, or nothing | Zig uses structural/duck typing where needed, not nominal interfaces. |
| `async/await` | No built-in async | Games are synchronous loops; concurrency is explicit (threads). |
| Python dynamic types | Everything has a type at compile time | No runtime type checks. |
| "objects are references" | Slices/pointers are explicit | Values are copied by default; `&x` / `*T` / `[]T` are written out. |

## Name-by-name translations

| Concept | TS/C#/Java/Python | Zig | Notes |
|---|---|---|---|
| variable | `let x` / `var x` | `const x` / `var x` | `const` = immutable binding; `var` = mutable. |
| sized int | `number` | `i8,u8,i16,u16,i32,u32,i64,u64` | pick a size. `i`=signed, `u`=unsigned. |
| float | `number` | `f32`/`f64` | |
| bool | `boolean`/`bool` | `bool` | |
| string | `string`/`str` | `[]const u8` | a *slice of bytes*. |
| array | `number[]` | `[N]T` (fixed) / `[]T` (slice) | slice = pointer + length. |
| map/dict | `Map<K,V>` / `dict` | `std.StringHashMap(V)` / `std.AutoHashMap(K,V)` | |
| optional | `T \| null`, `Maybe<T>`, `Optional` | `?T` | |
| error | `throw` | `error{Foo}` / `!T` | |
| function | `fn(x)` | `fn(x: i32) i32` | return type *after* `)`. |
| C function | (n/a) | `extern fn` / `callconv(.c)` | call an existing `.c`/`.so` function. |
| struct | `class`/`record`/`class` | `struct` or `extern struct` | `extern` = exact C memory layout. |
| enum | `enum` | `enum` or `enum(c_int)` | `enum(c_int)` = C-compatible enum. |
| if/else | same | `if/else if/else` | condition needs no parentheses. |
| switch | `switch` | `switch` | exhaustive, no fallthrough. |
| for-loop | `for (i=0;..)` | `for (items) \|it\|` or `while` | no C-style `for(;;)`. |
| null-check | `if (x != null)` | `if (x) \|v\|` | auto-unwraps the optional. |
| try/catch/finally | `try/catch` | `try` + `catch` + `defer` | `defer` ≈ finally. |
| object method | `obj.method()` | `Type.method(obj)` or `obj.method()` | Zig has namespaced/method-like calls. |
| import | `import`/`using`/`require` | `@import("std")` | `@import` + `@cImport`. |

## Worked example: "pick a random int, format a reply"

**TypeScript**
```ts
function reply(guess: number): string {
    const secret = 40;
    const delta = Math.abs(guess - secret);
    return delta === 0 ? "Correct!" : delta < 5 ? "Hot!" : "Cold.";
}
```

**Zig**
```zig
fn reply(guess: u32, secret: u32) []const u8 {
    if (guess == secret) return "Correct!";
    const delta = if (guess > secret) guess - secret else secret - guess;
    if (delta < 5) return "Hot!";
    return "Cold.";
}
```
Note: no `Math.abs` — you compute it explicitly (and watch for underflow!). Everything is explicit; that's the lesson.

## Where newbies trip on Zig

1. **`const` doesn't mean "constant value," it means "can't reassign."** An array declared `const` is still mutable *inside*; you just can't point `x` at a different array.
2. **No implicit casts.** `i32 + u32` won't compile. Use `@intCast` / `@floatCast` explicitly.
3. **Slices vs arrays.** `"abc"` is `*const [3:0]u8` (a sentinel-terminated pointer), and you usually want `"abc"[0..]` to get a `[]const u8`.
4. **`defer` runs at end of scope**, not immediately after the line. It runs even on error returns.
5. **`while |capture|`** changes scope: the captured name is only visible inside the block, and it *isn't null* there.
6. **`std` is huge but explicit.** You import exactly what you need; `.seconds()` etc. are namespaced, not methods on numbers.
