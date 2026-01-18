# Lesson 02: Variables & Constants

Learn C++ type system fundamentals by exploring how YaPB configures bot behavior.

## Learning Objectives

- Understand primitive types vs TypeScript types
- Learn the difference between `const` and `constexpr`
- Understand sized integer types (`int32_t`, `uint16_t`)
- Modify bot configuration values

## Theory: Primitive Types

### TypeScript Types (Runtime)

```typescript
let count: number = 10;      // All numbers are 64-bit floats
let name: string = "bot";    // Unicode strings
let active: boolean = true;  // true/false
```

TypeScript has only `number` for all numeric values. Type annotations are for tooling - they're erased at runtime.

### C++ Types (Compile Time)

```cpp
int count = 10;              // Signed integer (typically 32-bit)
float ratio = 0.5f;          // 32-bit floating point
double precise = 0.5;        // 64-bit floating point
bool active = true;          // true/false
char letter = 'A';           // Single character
```

C++ types are fixed at compile time and directly map to memory sizes.

### Type Size Comparison

| TypeScript | C++ | Size | Range |
|------------|-----|------|-------|
| `number` | `float` | 4 bytes | ~7 decimal digits |
| `number` | `double` | 8 bytes | ~15 decimal digits |
| `number` | `int` | 4 bytes | -2B to +2B |
| `boolean` | `bool` | 1 byte | true/false |
| `string` | `char*` | varies | C-style string |

### Sized Integer Types

For precise control, C++ offers sized types:

```cpp
#include <cstdint>

int8_t   tiny = 127;          // -128 to 127
uint8_t  byte = 255;          // 0 to 255
int16_t  small = 32767;       // -32768 to 32767
uint16_t word = 65535;        // 0 to 65535
int32_t  normal = 2147483647; // -2B to +2B
uint32_t large = 4294967295;  // 0 to 4B
```

YaPB uses these extensively for memory efficiency and cross-platform consistency.

## Theory: const vs constexpr

### TypeScript const

```typescript
const MAX_BOTS = 32;              // Immutable binding
const config = { maxBots: 32 };   // Object is mutable!
config.maxBots = 64;              // This works!
```

TypeScript `const` prevents reassignment but doesn't make values truly constant.

### C++ const

```cpp
const int MAX_BOTS = 32;          // Runtime constant
const int* ptr = &value;          // Pointer to constant (can't change value)
int* const ptr = &value;          // Constant pointer (can't change address)
```

`const` means the value won't change after initialization, but initialization can happen at runtime.

### C++ constexpr

```cpp
constexpr int MAX_BOTS = 32;      // Compile-time constant
constexpr float PI = 3.14159f;    // Known at compile time
constexpr int doubled = MAX_BOTS * 2;  // Computed at compile time
```

`constexpr` values are evaluated during compilation. They can be used in array sizes, template parameters, etc.

### When to Use Which

| Scenario | Use | Example |
|----------|-----|---------|
| Value known at compile time | `constexpr` | `constexpr int kMaxNodes = 4096` |
| Value set once at runtime | `const` | `const float startTime = getTime()` |
| Value may change | neither | `float health = 100.0f` |

## Code Study: YaPB Constants

### File: `inc/graph.h:10-11`

```cpp
constexpr int kMaxNodes = 4096;     // max nodes per graph
constexpr int kMaxNodeLinks = 8;    // max links for single node
```

These define navigation graph limits. Using `constexpr`:
- Values are known at compile time
- Can be used for array sizes
- Compiler can optimize code using these values

### File: `inc/constant.h` (excerpt)

```cpp
constexpr auto kInfiniteDistance = 9999999.0f;
constexpr auto kGrenadeCheckTime = 0.6f;
constexpr auto kSprayDistance = 272.0f;

constexpr auto kMaxWeapons = 32;
constexpr auto kNumWeapons = 26;
constexpr auto kGameMaxPlayers = 32;
```

Notice the naming convention:
- `k` prefix indicates a constant
- CamelCase for readability
- Descriptive names

### File: `inc/yapb.h:953-954`

```cpp
extern ConVar cv_camping_time_min;
extern ConVar cv_camping_time_max;
```

These aren't `constexpr` - they're console variables (cvars) that can be changed by users at runtime.

## Code Study: ConVar System

YaPB uses a `ConVar` (console variable) system for runtime configuration:

### File: `src/combat.cpp:10-11`

```cpp
ConVar cv_shoots_thru_walls ("shoots_thru_walls", "2",
    "Specifies whether bots can fire at enemies behind walls.",
    true, 0.0f, 3.0f);
```

This creates a variable that:
- Can be changed via console: `yb shoots_thru_walls 1`
- Has a default value: `"2"`
- Has min/max bounds: `0.0f` to `3.0f`

### TypeScript Equivalent

```typescript
interface ConVar {
  name: string;
  value: string;
  description: string;
  min?: number;
  max?: number;
}

const cv_shoots_thru_walls: ConVar = {
  name: "shoots_thru_walls",
  value: "2",
  description: "Specifies whether bots can fire at enemies behind walls.",
  min: 0,
  max: 3
};
```

## Exercise: Modify Camping Time

Bots use camping time cvars to decide how long to camp. Let's trace how these work.

### Your Task

1. Find `cv_camping_time_min` and `cv_camping_time_max` declarations
2. Find where they're defined with default values
3. Understand how they're used in the code
4. Test changing them via console

### Step-by-Step

#### Step 1: Find Declarations

Search in `inc/yapb.h`:
```bash
grep -n "cv_camping_time" inc/yapb.h
```

You'll find around line 953-954:
```cpp
extern ConVar cv_camping_time_min;
extern ConVar cv_camping_time_max;
```

`extern` means "this variable is defined elsewhere" - it's a declaration, not a definition.

#### Step 2: Find Definitions

Search for where they're actually created:
```bash
grep -rn "cv_camping_time" src/
```

In `src/tasks.cpp` you'll find the definitions with default values.

#### Step 3: Understand Usage

Find where these values are read:
```bash
grep -rn "cv_camping_time" --include="*.cpp"
```

Look for patterns like:
```cpp
cv_camping_time_min.as<float>()
```

### Console Test

Start CS 1.6 and try:

```
yb camping_time_min
yb camping_time_min 5
yb camping_time_max 10
```

Watch how bots change their camping behavior.

## Solution

### Where to Find Everything

**Declaration** (`inc/yapb.h:953-954`):
```cpp
extern ConVar cv_camping_time_min;
extern ConVar cv_camping_time_max;
```

**Definition** (`src/tasks.cpp`, near top):
```cpp
ConVar cv_camping_time_min ("camping_time_min", "15.0",
    "Lower bound of camping time.");
ConVar cv_camping_time_max ("camping_time_max", "45.0",
    "Upper bound of camping time.");
```

**Usage** (`src/tasks.cpp`, in `camp_` function):
```cpp
float campTime = rg (cv_camping_time_min.as<float>(),
                     cv_camping_time_max.as<float>());
m_timeCamping = game.time() + campTime;
```

The bot picks a random time between min and max for camping.

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Primitive Types | `int`, `float`, `bool` - fixed size |
| Sized Types | `int32_t`, `uint16_t` - explicit sizes |
| `const` | Runtime constant |
| `constexpr` | Compile-time constant |
| `extern` | Declaration of variable defined elsewhere |
| ConVar | Runtime-configurable console variable |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript
const MAX = 100;              →  constexpr int MAX = 100;
let count: number = 0;        →  int count = 0;
let ratio: number = 0.5;      →  float ratio = 0.5f;
let active: boolean = true;   →  bool active = true;
let name: string = "bot";     →  const char* name = "bot";
```

## Common Mistakes

### Missing 'f' Suffix for Floats

```cpp
// Warning - implicit double to float conversion
float ratio = 0.5;

// Correct - explicit float literal
float ratio = 0.5f;
```

### Using constexpr for Runtime Values

```cpp
// Error - can't compute at compile time
constexpr float startTime = game.time();

// Correct - use const for runtime
const float startTime = game.time();
```

### Integer Overflow

```cpp
int8_t small = 127;
small = small + 1;    // Overflow! Wraps to -128

// Use larger type if needed
int16_t bigger = 127;
bigger = bigger + 1;  // 128, no problem
```

## What's Next

In Lesson 03, you'll learn about functions and control flow:

- Function syntax and return types
- `if`/`else` and `switch` statements
- How YaPB's combat functions work

**[Next: Lesson 03 - Functions & Control Flow](../03-functions-control-flow/README.md)**
