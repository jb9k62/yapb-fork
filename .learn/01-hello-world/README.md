# Lesson 01: Hello World - Build System & Console Output

Your first modification to YaPB: adding a startup message that proves your code is running.

## Learning Objectives

- Understand compilation vs interpretation
- Learn about shared libraries (`.so`/`.dll`)
- Understand the `#include` directive
- Find and modify the plugin entry point
- Print messages to the game console

## Theory: TypeScript vs C++ Execution

### TypeScript (Interpreted/JIT)

```typescript
// app.ts
console.log("Hello");

// Run directly:
// ts-node app.ts
// or transpile + run:
// tsc && node app.js
```

TypeScript is either interpreted at runtime or transpiled to JavaScript. Either way, you're running through an interpreter.

### C++ (Compiled to Native)

```cpp
// hello.cpp
#include <iostream>
int main() {
    std::cout << "Hello" << std::endl;
    return 0;
}

// Compile to native binary:
// clang++ hello.cpp -o hello
// ./hello
```

C++ compiles directly to machine code. No interpreter needed - the CPU runs your code directly.

### Why This Matters

| Aspect | TypeScript | C++ |
|--------|------------|-----|
| Startup | Slow (interpreter loads) | Fast (direct execution) |
| Runtime | Interpreter overhead | Native speed |
| Errors | Runtime errors common | Most errors at compile time |
| Memory | Garbage collected | Manual/RAII |

For game plugins, native speed is essential - bots think 100+ times per second.

## Theory: Shared Libraries

YaPB compiles to a **shared library** (`.so` on Linux, `.dll` on Windows), not an executable.

### Executables vs Shared Libraries

```
Executable (.exe, no extension)
├── Has main() entry point
├── Runs independently
└── Example: Counter-Strike (hl)

Shared Library (.so, .dll)
├── No main() - loaded by another program
├── Exports functions for the host to call
└── Example: YaPB (yapb.so)
```

### How YaPB Gets Loaded

```
1. User starts Counter-Strike
2. Engine reads liblist.gam
3. Engine loads yapb.so
4. Engine calls GiveFnptrsToDll() in yapb.so
5. YaPB initializes and hooks into game events
```

## Theory: The #include Directive

In TypeScript, you import modules:

```typescript
import { Bot } from './bot';
```

In C++, you `#include` headers:

```cpp
#include <yapb.h>
```

### Key Differences

| TypeScript `import` | C++ `#include` |
|---------------------|----------------|
| Module system | Text substitution |
| Only imports what's listed | Includes entire file |
| Modules loaded once | Can include multiple times |
| Affects runtime | Affects compilation only |

`#include` literally copies the file contents at that point. It's a preprocessor directive (runs before compilation).

### Include Syntax

```cpp
#include <stdio.h>     // System/library headers (angle brackets)
#include "myheader.h"  // Project headers (quotes)
#include <yapb.h>      // YaPB uses angle brackets for project headers too
```

## Code Study: Entry Point

Let's examine where YaPB starts executing.

### File: `src/linkage.cpp:1072-1105`

Open `src/linkage.cpp` and find `GiveFnptrsToDll`:

```cpp
DLL_GIVEFNPTRSTODLL GiveFnptrsToDll (enginefuncs_t *table, globalvars_t *glob) {
   // this is the very first function that is called in the game DLL by the game.

   // get the engine functions from the game...
   memcpy (&engfuncs, table, sizeof (enginefuncs_t));
   globals = glob;

   if (game.postload ()) {
      return;
   }
   // ... more initialization
}
```

### Understanding the Signature

```cpp
DLL_GIVEFNPTRSTODLL GiveFnptrsToDll (enginefuncs_t *table, globalvars_t *glob)
│                   │                │                     │
│                   │                │                     └── Global game state
│                   │                └── Engine function pointers
│                   └── Function name (required by engine)
└── Export macro (makes function visible to engine)
```

- `enginefuncs_t *table` - Pointers to engine functions we can call
- `globalvars_t *glob` - Shared global variables

### The Engine Function Table

After `GiveFnptrsToDll` runs, we have `engfuncs` available globally. This lets us call engine functions:

```cpp
engfuncs.pfnServerPrint("Hello from YaPB!\n");  // Print to console
engfuncs.pfnCreateEntity();                      // Create game entity
engfuncs.pfnClientCommand(ent, "say hi");       // Make player say something
```

## Exercise: Add a Startup Message

Add a custom message that prints when YaPB loads.

### Your Task

1. Open `src/linkage.cpp`
2. Find the `GiveFnptrsToDll` function (line ~1072)
3. After `memcpy (&engfuncs, table, sizeof (enginefuncs_t));`
4. Add your message using `engfuncs.pfnServerPrint()`

### Hints

- `pfnServerPrint` needs a newline (`\n`) at the end
- Put your code after `engfuncs` is populated but before other code
- C strings use double quotes: `"Hello"`

### Build and Test

```bash
# Compile
meson compile -C build

# Deploy (using script from Lesson 00)
./deploy.sh

# Start CS 1.6, open console (~), look for your message
```

## Solution

Here's the complete solution:

```cpp
// In src/linkage.cpp, inside GiveFnptrsToDll function
// After line: memcpy (&engfuncs, table, sizeof (enginefuncs_t));

DLL_GIVEFNPTRSTODLL GiveFnptrsToDll (enginefuncs_t *table, globalvars_t *glob) {
   // get the engine functions from the game...
   memcpy (&engfuncs, table, sizeof (enginefuncs_t));
   globals = glob;

   // YOUR CODE: Print a startup message
   engfuncs.pfnServerPrint("*** HELLO FROM MY YAPB BUILD! ***\n");

   if (game.postload ()) {
      return;
   }
   // ... rest of function
}
```

### Solution File

See `solution.cpp` for the exact code snippet.

## Verification

After building and deploying:

1. Start Counter-Strike 1.6
2. Press `~` to open console
3. You should see your message near the top:
   ```
   *** HELLO FROM MY YAPB BUILD! ***
   [YAPB] YaPB vX.X.X (Build: XXX)
   ```

If you see your message, congratulations! You've successfully modified a C++ game plugin.

## Deep Dive: What Just Happened

### Compilation Steps

```
1. Preprocessor
   #include <yapb.h>  →  [copies 10000+ lines of code]

2. Compilation
   .cpp files  →  .o object files (machine code)

3. Linking
   .o files  →  yapb.so (shared library)
```

### Why No main()?

Shared libraries don't have `main()`. Instead, they export functions that the host program calls. `GiveFnptrsToDll` is YaPB's "entry point" - the first function the engine calls.

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Compilation | Source code → native machine code |
| Shared Library | Code loaded by another program |
| `#include` | Preprocessor text substitution |
| Entry Point | First function called (`GiveFnptrsToDll`) |
| Engine Functions | `engfuncs.pfn*` for engine interaction |

## Common Mistakes

### Forgetting the Newline

```cpp
// Wrong - output may be garbled
engfuncs.pfnServerPrint("Hello");

// Correct - always end with \n
engfuncs.pfnServerPrint("Hello\n");
```

### Using cout/printf

```cpp
// Won't work - stdout isn't the game console
printf("Hello\n");           // Goes nowhere
std::cout << "Hello";        // Goes nowhere

// Correct - use engine function
engfuncs.pfnServerPrint("Hello\n");
```

### Code Before engfuncs is Populated

```cpp
// Wrong - engfuncs not initialized yet!
engfuncs.pfnServerPrint("Hello\n");
memcpy (&engfuncs, table, sizeof (enginefuncs_t));

// Correct - after memcpy
memcpy (&engfuncs, table, sizeof (enginefuncs_t));
engfuncs.pfnServerPrint("Hello\n");
```

## Exercises for Extra Practice

1. **Change the message**: Add your name or date
2. **Multiple messages**: Print 2-3 lines
3. **Find other print locations**: Search for `pfnServerPrint` in the codebase

## What's Next

In Lesson 02, you'll learn about C++ variables and constants, including:

- Primitive types (`int`, `float`, `bool`)
- `const` vs `constexpr`
- Sized integer types (`int32_t`)
- How YaPB uses constants for configuration

**[Next: Lesson 02 - Variables & Constants](../02-variables-constants/README.md)**
