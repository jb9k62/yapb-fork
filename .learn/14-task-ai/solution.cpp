// Solution for Lesson 14: Task-Based AI
//
// Task transition logging implementation.

// ============================================================
// HELPER: Get task name string
// ============================================================

const char* getTaskName(Task id) {
    switch (id) {
        case Task::Normal:          return "Normal";
        case Task::Pause:           return "Pause";
        case Task::MoveToPosition:  return "MoveToPos";
        case Task::FollowUser:      return "FollowUser";
        case Task::PickupItem:      return "PickupItem";
        case Task::Camp:            return "Camp";
        case Task::PlantBomb:       return "PlantBomb";
        case Task::DefuseBomb:      return "DefuseBomb";
        case Task::Attack:          return "Attack";
        case Task::Hunt:            return "Hunt";
        case Task::SeekCover:       return "SeekCover";
        case Task::ThrowExplosive:  return "ThrowHE";
        case Task::ThrowFlashbang:  return "ThrowFlash";
        case Task::ThrowSmoke:      return "ThrowSmoke";
        case Task::DoubleJump:      return "DoubleJump";
        case Task::EscapeFromBomb:  return "EscapeBomb";
        case Task::ShootBreakable:  return "Breakable";
        case Task::Hide:            return "Hide";
        case Task::Blind:           return "Blind";
        case Task::Spraypaint:      return "Spray";
        default:                    return "Unknown";
    }
}

// ============================================================
// MODIFIED executeTasks with logging
// ============================================================

void Bot::executeTasks() {
    // Track last task per bot (static array)
    static Task lastTask[kGameMaxPlayers] = {};
    static float lastLogTime[kGameMaxPlayers] = {};

    BotTask* task = getTask();
    float currentTime = game.time();

    // Check for task change
    if (task->id != lastTask[m_index]) {
        // Avoid spamming (min 0.5s between logs for same bot)
        if (currentTime - lastLogTime[m_index] > 0.5f) {
            ctrl.msg("[AI] %s: %s → %s (pri: %.1f)",
                     pev->netname.chars(),
                     getTaskName(lastTask[m_index]),
                     getTaskName(task->id),
                     task->desire);
            lastLogTime[m_index] = currentTime;
        }
        lastTask[m_index] = task->id;
    }

    // Execute the current task
    (this->*task->func)();
}

// ============================================================
// DETAILED TASK DEBUGGING
// ============================================================

void Bot::debugTasks() {
    ctrl.msg("%s Task Stack:", pev->netname.chars());

    for (const auto& task : m_tasks) {
        const char* active = (&task == getTask()) ? " [ACTIVE]" : "";
        ctrl.msg("  %s (%.1f)%s",
                 getTaskName(task.id),
                 task.desire,
                 active);

        // Extra info based on task type
        if (task.data != -1) {
            ctrl.msg("    Data: %d", task.data);
        }
        if (task.time > 0) {
            float remaining = task.time - game.time();
            if (remaining > 0) {
                ctrl.msg("    Expires in: %.1fs", remaining);
            }
        }
    }
}

// ============================================================
// CHATTER ON TASK CHANGE
// ============================================================

void Bot::handleChatterTaskChange(Task newTask) {
    // Bot can "announce" certain task changes via chatter
    switch (newTask) {
        case Task::Camp:
            pushChatterMessage(Chatter::GoingToCamp);
            break;
        case Task::DefuseBomb:
            pushChatterMessage(Chatter::DefusingBomb);
            break;
        case Task::PlantBomb:
            pushChatterMessage(Chatter::PlantingBomb);
            break;
        case Task::Attack:
            pushChatterMessage(Chatter::InCombat);
            break;
        default:
            break;
    }
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class Bot {
    private tasks: BotTask[] = [];
    private lastTask: Task = Task.Normal;

    executeTasks(): void {
        const task = this.getHighestPriorityTask();

        // Log task changes
        if (task.id !== this.lastTask) {
            console.log(
                `[AI] ${this.name}: ${Task[this.lastTask]} → ${Task[task.id]} ` +
                `(pri: ${task.desire.toFixed(1)})`
            );
            this.lastTask = task.id;
        }

        // Execute task
        task.execute.call(this);
    }

    getHighestPriorityTask(): BotTask {
        return this.tasks.reduce((best, task) =>
            task.desire > best.desire ? task : best
        );
    }

    debugTasks(): void {
        console.log(`${this.name} Task Stack:`);
        const active = this.getHighestPriorityTask();

        for (const task of this.tasks) {
            const marker = task === active ? " [ACTIVE]" : "";
            console.log(`  ${Task[task.id]} (${task.desire})${marker}`);
        }
    }
}
*/

// ============================================================
// TASK FLOW EXAMPLES
// ============================================================

/*
Normal gameplay flow:

1. Bot spawns
   Tasks: [Normal(35)]
   Active: Normal

2. Bot sees enemy
   setConditions() detects enemy
   startTask(Attack, 90)
   Tasks: [Normal(35), Attack(90)]
   Active: Attack

3. Enemy dies
   clearTask(Attack)
   Tasks: [Normal(35)]
   Active: Normal

4. Bot gets flashed
   takeBlind() called
   startTask(Blind, 100)
   Tasks: [Normal(35), Blind(100)]
   Active: Blind

5. Flash wears off
   Task expires (time check)
   Tasks: [Normal(35)]
   Active: Normal
*/

// ============================================================
// CONSOLE COMMAND for debugging
// ============================================================

// In src/control.cpp:
if (isCommand("tasks")) {
    for (auto &bot : bots) {
        bot->debugTasks();
    }
    return true;
}
