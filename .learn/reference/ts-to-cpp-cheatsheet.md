# TypeScript to C++ Cheatsheet

Quick reference for TypeScript developers learning C++ through YaPB.

## Variables & Types

| TypeScript | C++ | Notes |
|------------|-----|-------|
| `let x: number = 5` | `int x = 5` | Integer |
| `let x: number = 5.5` | `float x = 5.5f` | Note the `f` suffix |
| `const x = 10` | `const int x = 10` | Runtime constant |
| `const x = 10` | `constexpr int x = 10` | Compile-time constant |
| `let s: string = "hi"` | `const char* s = "hi"` | C-style string |
| `let s: string = "hi"` | `String s = "hi"` | YaPB String class |
| `let b: boolean = true` | `bool b = true` | Boolean |
| `let arr: number[] = []` | `Array<int> arr` | Dynamic array |

## Type Sizes

| C++ Type | Size | Range |
|----------|------|-------|
| `int8_t` | 1 byte | -128 to 127 |
| `uint8_t` | 1 byte | 0 to 255 |
| `int16_t` | 2 bytes | -32K to 32K |
| `int32_t` | 4 bytes | -2B to 2B |
| `float` | 4 bytes | ~7 digits precision |
| `double` | 8 bytes | ~15 digits precision |

## Functions

```typescript
// TypeScript
function add(a: number, b: number): number {
    return a + b;
}

const greet = (name: string): void => {
    console.log(`Hello ${name}`);
};
```

```cpp
// C++
int add(int a, int b) {
    return a + b;
}

void greet(const char* name) {
    ctrl.msg("Hello %s", name);
}
```

## Classes

```typescript
// TypeScript
class Bot {
    private health: number;
    public name: string;

    constructor(name: string) {
        this.name = name;
        this.health = 100;
    }

    takeDamage(amount: number): void {
        this.health -= amount;
    }

    getHealth(): number {
        return this.health;
    }
}
```

```cpp
// C++ Header (bot.h)
class Bot {
private:
    float m_health;

public:
    String m_name;

    Bot(const char* name);
    void takeDamage(float amount);
    float getHealth() const;
};

// C++ Implementation (bot.cpp)
Bot::Bot(const char* name)
    : m_name(name), m_health(100.0f) {}

void Bot::takeDamage(float amount) {
    m_health -= amount;
}

float Bot::getHealth() const {
    return m_health;
}
```

## Null/Undefined

```typescript
// TypeScript
let enemy: Entity | null = null;
let value: number | undefined;

if (enemy !== null) {
    enemy.attack();
}
```

```cpp
// C++
edict_t* enemy = nullptr;
// No equivalent to undefined

if (enemy != nullptr) {  // or just: if (enemy)
    enemy->v.health -= 10;
}
```

## Arrays/Collections

```typescript
// TypeScript
const arr: number[] = [1, 2, 3];
arr.push(4);
const first = arr[0];
const len = arr.length;

for (const item of arr) {
    console.log(item);
}
```

```cpp
// C++
Array<int> arr = {1, 2, 3};
arr.push(4);
int first = arr[0];
int len = arr.length();

for (const auto& item : arr) {
    ctrl.msg("%d", item);
}
```

## Pointers vs References

```typescript
// TypeScript - all objects are references
let bot1 = new Bot();
let bot2 = bot1;  // Both point to same object
bot2.health = 50; // bot1.health also 50
```

```cpp
// C++ - explicit control
Bot bot1;
Bot bot2 = bot1;    // COPY (different objects)
Bot& ref = bot1;    // REFERENCE (alias)
Bot* ptr = &bot1;   // POINTER (address)

ref.health = 50;    // Modifies bot1
ptr->health = 50;   // Also modifies bot1 (via pointer)
```

## Memory Access

```typescript
// TypeScript
obj.property        // Always dot notation
```

```cpp
// C++
obj.property        // For values/references
ptr->property       // For pointers (same as (*ptr).property)
```

## Control Flow

```typescript
// TypeScript
if (x > 0) { } else if (x < 0) { } else { }

switch (value) {
    case 1: break;
    default: break;
}

for (let i = 0; i < 10; i++) { }
for (const item of array) { }
while (condition) { }
```

```cpp
// C++ - Nearly identical!
if (x > 0) { } else if (x < 0) { } else { }

switch (value) {
    case 1: break;
    default: break;
}

for (int i = 0; i < 10; i++) { }
for (const auto& item : array) { }
while (condition) { }
```

## String Formatting

```typescript
// TypeScript
const msg = `Player ${name} has ${health} HP`;
console.log(msg);
```

```cpp
// C++
ctrl.msg("Player %s has %.0f HP", name, health);
// %s = string, %d = int, %f = float, %.0f = float no decimals
```

## Enums

```typescript
// TypeScript
enum Team {
    Terrorist = 0,
    CT = 1
}
const team: Team = Team.Terrorist;
```

```cpp
// C++
CR_DECLARE_SCOPED_ENUM(Team,
    Terrorist = 0,
    CT = 1
)
Team team = Team::Terrorist;  // Note: :: not .
```

## Imports/Includes

```typescript
// TypeScript
import { Bot } from './bot';
import * as utils from './utils';
```

```cpp
// C++
#include <bot.h>      // Angle brackets for system/lib
#include "utils.h"    // Quotes for project files
```

## Async/Promises

TypeScript has built-in async support:
```typescript
async function load(): Promise<Data> {
    const result = await fetch(url);
    return result.json();
}
```

C++ has no built-in equivalent. YaPB uses:
- Callbacks
- Frame-based execution
- Worker threads (for heavy operations)

## Common Gotchas

### 1. Semicolons Required
```cpp
int x = 5     // ERROR!
int x = 5;    // Correct
```

### 2. Float Literals Need 'f'
```cpp
float x = 3.14;   // Warning: double to float
float x = 3.14f;  // Correct
```

### 3. No Implicit Type Coercion
```cpp
int a = 5;
float b = a / 2;    // b = 2.0 (integer division!)
float b = a / 2.0f; // b = 2.5 (correct)
```

### 4. Strings Are Not Objects
```cpp
// TypeScript
"hello".length;     // 5
"hello".toUpperCase();

// C++
strlen("hello");              // For length
// No built-in methods on string literals
```

### 5. Return Type Before Function Name
```cpp
// TypeScript
function add(a: number, b: number): number

// C++
int add(int a, int b)  // Return type first!
```

## YaPB-Specific Patterns

### Global Singletons
```cpp
game.time();           // Game singleton
bots.getBotCount();    // BotManager singleton
graph.length();        // BotGraph singleton
ctrl.msg("...");       // BotControl singleton
```

### Entity Access
```cpp
edict_t* ent = ...;
ent->v.health          // Entity health
ent->v.origin          // Entity position
ent->v.netname.chars() // Entity name (as C string)
```

### Vector Math
```cpp
Vector a(1, 2, 3);
Vector b(4, 5, 6);

Vector sum = a + b;         // Addition
Vector diff = a - b;        // Subtraction
float dot = a | b;          // Dot product
Vector cross = a ^ b;       // Cross product
float len = a.length();     // Length
float dist = a.distance(b); // Distance
```
