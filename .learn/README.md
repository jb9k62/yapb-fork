# YaPB C++ Learning Curriculum

A structured learning path for TypeScript/Node.js developers to learn C++ and game/systems programming through the YaPB codebase.

## Who This Is For

- Experienced TypeScript/Node.js developers (5+ years)
- Little formal CS theory background
- Interested in CS 1.6 modding and game programming
- Want practical, hands-on learning with visible results

## What You'll Learn

By completing this curriculum, you will:

- Understand C++ fundamentals from a TypeScript perspective
- Learn memory management (pointers, RAII, smart pointers)
- Master the GoldSrc engine plugin architecture
- Understand game loop programming and frame-based execution
- Learn AI programming patterns (state machines, pathfinding)
- Build confidence to modify and extend the YaPB codebase

## Prerequisites

Before starting, ensure you have:

1. **Counter-Strike 1.6** installed (Steam version recommended)
2. **Build tools**: Meson, Ninja, Clang (see `00-setup/` for details)
3. **Code editor**: VS Code with C/C++ extension recommended
4. **Git** for version control
5. Basic command line familiarity

## Curriculum Structure

### Phase 1: Foundations (Lessons 1-3)
Build your first modifications and understand C++ basics.

| Lesson | Topic | Key Concepts |
|--------|-------|--------------|
| 01 | Hello World | Build system, console output, `#include` |
| 02 | Variables & Constants | Types, `const`, `constexpr`, sized integers |
| 03 | Functions & Control Flow | Function syntax, if/switch, return types |

### Phase 2: Memory & Pointers (Lessons 4-6)
Understand how C++ manages memory differently than JavaScript.

| Lesson | Topic | Key Concepts |
|--------|-------|--------------|
| 04 | Structs & Data | Memory layout, struct vs interface |
| 05 | Pointers & References | Addresses, dereferencing, `nullptr` |
| 06 | Smart Pointers & RAII | Stack vs heap, `UniquePtr`, ownership |

### Phase 3: OOP & Patterns (Lessons 7-9)
Learn object-oriented patterns used throughout the codebase.

| Lesson | Topic | Key Concepts |
|--------|-------|--------------|
| 07 | Classes & OOP | Header/impl split, constructors, destructors |
| 08 | Singleton Pattern | Global access, `CR_EXPOSE_GLOBAL_SINGLETON` |
| 09 | Inheritance & Virtual | `final`, `override`, why YaPB avoids inheritance |

### Phase 4: Game Systems (Lessons 10-12)
Understand how the game engine and bot interact.

| Lesson | Topic | Key Concepts |
|--------|-------|--------------|
| 10 | The Game Loop | `StartFrame`, timing, frame-based execution |
| 11 | Entity System | `edict_t`, `entvars_t`, entity indexing |
| 12 | Vector Math & 3D Space | 3D vectors, distance, dot product |

### Phase 5: Advanced AI (Lessons 13-15)
Dive into the AI systems that make bots intelligent.

| Lesson | Topic | Key Concepts |
|--------|-------|--------------|
| 13 | Navigation & Pathfinding | A* algorithm, node graphs, heuristics |
| 14 | Task-Based AI | State machines, desire/priority system |
| 15 | Performance & Optimization | Frame budgeting, throttling, profiling |

## How to Use This Curriculum

### Lesson Structure

Each lesson contains:

1. **Theory** - Concepts explained with TypeScript comparisons
2. **Code References** - Exact file:line locations in YaPB
3. **Exercise** - Hands-on modification to make
4. **Solution** - Complete working code to reference
5. **Verification** - How to test your changes work

### Recommended Workflow

```bash
# 1. Read the theory section
# 2. Explore the referenced code in your editor
# 3. Attempt the exercise yourself
# 4. Compare with the solution if stuck
# 5. Build and test in CS 1.6
# 6. Move to the next lesson
```

### Time Estimate

- Each lesson: 30-60 minutes
- Full curriculum: 10-15 hours
- Pace yourself - understanding beats speed

## Quick Reference

The `reference/` folder contains:

- **ts-to-cpp-cheatsheet.md** - Side-by-side TypeScript to C++ translations
- **goldsrc-engine-overview.md** - How the Half-Life plugin system works

## Getting Started

Begin with the setup lesson:

```bash
cd .learn/00-setup/
cat README.md
```

This will guide you through:
1. Installing build dependencies
2. Compiling YaPB
3. Installing the bot in CS 1.6
4. Verifying everything works

## Tips for TypeScript Developers

### Mental Model Shifts

| TypeScript | C++ |
|------------|-----|
| Runtime type checking | Compile-time type checking |
| Garbage collected | Manual memory (or RAII) |
| `null`/`undefined` | `nullptr` (one concept) |
| Objects by reference | Value by default, pointers explicit |
| `interface` | `struct`/`class` (similar) |
| `async`/`await` | No built-in (threads/callbacks) |

### Common Gotchas

1. **Semicolons are mandatory** - Every statement needs one
2. **Braces don't create scope for `var`** - All variables are block-scoped like `let`
3. **No implicit type coercion** - `int + float` requires explicit cast
4. **Header vs Implementation** - Declarations go in `.h`, definitions in `.cpp`
5. **Compilation order matters** - Must `#include` before using

## Troubleshooting

### Build Issues

```bash
# Clean rebuild
rm -rf build
meson setup build
meson compile -C build
```

### Game Won't Load Bot

1. Check `liblist.gam` has correct path
2. Verify `yapb.so` exists in `addons/yapb/`
3. Check console for error messages

### Changes Don't Appear

1. Rebuild after each change: `meson compile -C build`
2. Copy new `.so` to game directory
3. Restart CS 1.6 (map change isn't enough for DLL reload)

## Contributing

Found an error or want to improve a lesson? The curriculum files are in `.learn/` - PRs welcome!

## Next Step

Ready to begin? Start with the setup:

**[Lesson 00: Setup](./00-setup/README.md)**
