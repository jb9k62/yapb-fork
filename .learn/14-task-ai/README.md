# Lesson 14: Task-Based AI

Understand how bots decide what to do using a task system with priorities and desires.

## Learning Objectives

- Understand state machines and tasks
- Learn the desire/priority system
- Trace task changes in bot behavior
- Add task transition logging

## Theory: Task-Based AI

Instead of complex behavior trees, YaPB uses a **task stack** with priorities.

### The Concept

```
Task Stack (highest priority at top):
┌────────────────────────┐
│ Blind (100.0) ← ACTIVE │  Currently blinded!
├────────────────────────┤
│ Attack (90.0)          │  Was fighting
├────────────────────────┤
│ Normal (35.0)          │  Base wandering
└────────────────────────┘
```

The bot always executes the task with highest desire/priority.

### TypeScript Equivalent

```typescript
interface Task {
    id: TaskId;
    desire: number;   // Priority (higher = more important)
    execute: () => void;
}

class Bot {
    tasks: Task[] = [];

    frame() {
        // Find highest priority task
        const activeTask = this.tasks.reduce((a, b) =>
            a.desire > b.desire ? a : b
        );
        activeTask.execute();
    }
}
```

## Code Study: BotTask Structure

### File: `inc/yapb.h:25-38`

```cpp
struct BotTask {
   using Function = void (Bot:: *) ();  // Method pointer

   Function func {};   // Function to execute (normal_, attack_, etc.)
   Task id {};         // Task identifier
   float desire {};    // Priority value
   int data {};        // Extra data (e.g., node index)
   float time {};      // Expiration time
   bool resume {};     // Can be resumed after interruption
};
```

### File: `inc/constant.h` (Task enum)

```cpp
CR_DECLARE_SCOPED_ENUM(Task,
   Normal = 0,
   Pause,
   MoveToPosition,
   FollowUser,
   PickupItem,
   Camp,
   PlantBomb,
   DefuseBomb,
   Attack,
   Hunt,
   SeekCover,
   // ... more tasks
)
```

### Priority Constants

```cpp
// File: inc/constant.h (TaskPri namespace)
namespace TaskPri {
   constexpr auto Normal { 35.0f };
   constexpr auto Camp { 37.0f };
   constexpr auto MoveToPosition { 50.0f };
   constexpr auto DefuseBomb { 89.0f };
   constexpr auto Attack { 90.0f };
   constexpr auto Blind { 100.0f };
   constexpr auto EscapeFromBomb { 100.0f };
}
```

Higher priority = more urgent.

## Code Study: Task Execution

### File: `src/tasks.cpp`

Each task has a dedicated function:

```cpp
void Bot::normal_() {
    // Default behavior: wander, look for enemies
    m_aimFlags |= AimFlags::Nav;
    // ... navigation logic
}

void Bot::attack_() {
    // Combat behavior
    m_aimFlags |= AimFlags::Enemy;
    fireWeapons();
    attackMovement();
}

void Bot::camp_() {
    // Camping behavior
    m_moveSpeed = 0.0f;
    m_strafeSpeed = 0.0f;
    // ... look around
}
```

### Task Selection

```cpp
void Bot::executeTasks() {
    // Get highest priority task
    BotTask* task = getTask();

    // Execute the task's function
    (this->*task->func)();
}

BotTask* Bot::getTask() {
    // Find task with highest desire
    BotTask* best = &m_tasks[0];
    for (auto& task : m_tasks) {
        if (task.desire > best->desire) {
            best = &task;
        }
    }
    return best;
}
```

## Code Study: Starting a Task

### File: `src/botlib.cpp`

```cpp
void Bot::startTask(Task id, float desire, int data, float time, bool resume) {
    // Find the function for this task
    Function func = getTaskFunction(id);

    // Add to task list
    m_tasks.emplace(BotTask{func, id, desire, data, time, resume});
}

// Example: Start attacking
startTask(Task::Attack, TaskPri::Attack, -1, 0.0f, true);

// Example: Move to position
startTask(Task::MoveToPosition, TaskPri::MoveToPosition, nodeIndex, 0.0f, true);
```

## Theory: Task Transitions

Tasks change based on conditions:

```
[Normal] ──see enemy──> [Attack]
    ↑                       │
    │                       │ enemy dead
    │                       ↓
    └───────────────── [Normal]


[Normal] ──bomb planted──> [DefuseBomb]
    ↑                           │
    │                           │ defused/exploded
    └───────────────────────────┘
```

### In Code

```cpp
void Bot::setConditions() {
    // Check for enemies
    if (lookupEnemies()) {
        m_states |= Sense::SeeingEnemy;
        startTask(Task::Attack, TaskPri::Attack, ...);
    }

    // Check for bomb
    if (needsToDefuseBomb()) {
        startTask(Task::DefuseBomb, TaskPri::DefuseBomb, ...);
    }
}
```

## Exercise: Add Task Transition Logging

Log when bots change tasks to understand AI behavior.

### Your Task

1. Track the current task
2. Log when task changes
3. Include task name and priority

### Implementation

Add to `src/tasks.cpp`:

```cpp
void Bot::executeTasks() {
    static Task lastTask[32] = {};  // Per-bot tracking

    BotTask* task = getTask();

    // Check for task change
    if (task->id != lastTask[m_index]) {
        const char* taskName = getTaskName(task->id);
        const char* lastName = getTaskName(lastTask[m_index]);

        ctrl.msg("%s: Task change %s → %s (desire: %.1f)",
                 pev->netname.chars(),
                 lastName,
                 taskName,
                 task->desire);

        lastTask[m_index] = task->id;
    }

    // Execute
    (this->*task->func)();
}

const char* Bot::getTaskName(Task id) {
    switch (id) {
        case Task::Normal:      return "Normal";
        case Task::Attack:      return "Attack";
        case Task::Camp:        return "Camp";
        case Task::DefuseBomb:  return "DefuseBomb";
        case Task::PlantBomb:   return "PlantBomb";
        case Task::Hunt:        return "Hunt";
        case Task::SeekCover:   return "SeekCover";
        // ... more cases
        default:                return "Unknown";
    }
}
```

### Build and Test

Watch the console during gameplay to see task transitions:
```
Alfred: Task change Normal → Attack (desire: 90.0)
Alfred: Task change Attack → Normal (desire: 35.0)
Boris: Task change Normal → Camp (desire: 37.0)
```

## Solution

See `solution.cpp` for the complete implementation.

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Task | Unit of behavior (attack, camp, etc.) |
| Desire | Priority value for task selection |
| Task stack | All active/potential tasks |
| `getTask()` | Returns highest priority task |
| `startTask()` | Adds new task to stack |
| Task function | `normal_()`, `attack_()`, etc. |

## What's Next

In Lesson 15, you'll learn about Performance Optimization:

- Frame budgeting
- Throttling expensive operations
- Profiling bot code

**[Next: Lesson 15 - Performance & Optimization](../15-performance/README.md)**
