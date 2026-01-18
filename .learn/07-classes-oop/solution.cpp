// Solution for Lesson 07: Classes & OOP
//
// This exercise adds a reportStatus method to the Bot class.

// ============================================================
// STEP 1: Add declaration in inc/yapb.h
// ============================================================
// In the Bot class public methods section (around line 760), add:

void reportStatus() const;

// ============================================================
// STEP 2: Add implementation in src/botlib.cpp
// ============================================================

void Bot::reportStatus() const {
    // Map task ID to human-readable name
    const char* taskName = "Unknown";

    switch (getCurrentTaskId()) {
        case Task::Normal:        taskName = "Normal";        break;
        case Task::Pause:         taskName = "Pause";         break;
        case Task::MoveToPosition:taskName = "MoveToPos";     break;
        case Task::FollowUser:    taskName = "FollowUser";    break;
        case Task::PickupItem:    taskName = "PickupItem";    break;
        case Task::Camp:          taskName = "Camp";          break;
        case Task::PlantBomb:     taskName = "PlantBomb";     break;
        case Task::DefuseBomb:    taskName = "DefuseBomb";    break;
        case Task::Attack:        taskName = "Attack";        break;
        case Task::Hunt:          taskName = "Hunt";          break;
        case Task::SeekCover:     taskName = "SeekCover";     break;
        case Task::ThrowExplosive:taskName = "ThrowHE";       break;
        case Task::ThrowFlashbang:taskName = "ThrowFlash";    break;
        case Task::ThrowSmoke:    taskName = "ThrowSmoke";    break;
        case Task::DoubleJump:    taskName = "DoubleJump";    break;
        case Task::EscapeFromBomb:taskName = "EscapeBomb";    break;
        case Task::ShootBreakable:taskName = "Breakable";     break;
        case Task::Hide:          taskName = "Hide";          break;
        case Task::Blind:         taskName = "Blind";         break;
        default:                  taskName = "Other";         break;
    }

    // Get team name
    const char* teamName = m_team == Team::Terrorist ? "T" : "CT";

    // Build status message
    ctrl.msg("[%s] %s | HP: %.0f | Task: %s | Node: %d | Goal: %d | Enemies: %d",
             teamName,                    // Team
             pev->netname.chars(),        // Bot name
             pev->health,                 // Health
             taskName,                    // Current task
             m_currentNodeIndex,          // Current navigation node
             m_chosenGoalIndex,           // Target goal node
             m_numEnemiesLeft);           // Enemies alive
}

// ============================================================
// STEP 3: Add periodic call in Bot::logic() (temporary test)
// ============================================================
// In src/botlib.cpp, find Bot::logic() and add at the start:

void Bot::logic() {
    // TEMPORARY: Report status every 5 seconds
    static HashMap<int, float> nextReportTime;
    float currentTime = game.time();

    if (!nextReportTime.exists(m_index) ||
        currentTime > nextReportTime[m_index]) {
        reportStatus();
        nextReportTime[m_index] = currentTime + 5.0f;
    }

    // ... rest of existing logic code ...
}

// ============================================================
// ALTERNATIVE: Console command version
// ============================================================
// You could also make this a console command in src/control.cpp:

// In the command handler:
if (isCommand("status")) {
    for (auto &bot : bots) {
        bot->reportStatus();
    }
    return true;
}

// Usage: yb status

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class Bot {
    private currentTaskId: Task;
    private currentNodeIndex: number;
    private team: Team;
    private health: number;
    private name: string;

    reportStatus(): void {
        const taskNames: Record<Task, string> = {
            [Task.Normal]: "Normal",
            [Task.Attack]: "Attack",
            [Task.Camp]: "Camp",
            // ...
        };

        const taskName = taskNames[this.currentTaskId] ?? "Unknown";
        const teamName = this.team === Team.Terrorist ? "T" : "CT";

        console.log(
            `[${teamName}] ${this.name} | ` +
            `HP: ${this.health} | ` +
            `Task: ${taskName} | ` +
            `Node: ${this.currentNodeIndex}`
        );
    }
}
*/

// ============================================================
// KEY OOP CONCEPTS DEMONSTRATED
// ============================================================

// 1. const method: void reportStatus() const
//    - The 'const' after () means this method doesn't modify the object
//    - Can only call other const methods
//    - Good practice for getters and reporters

// 2. Accessing members without 'this->':
//    - m_currentNodeIndex is equivalent to this->m_currentNodeIndex
//    - In C++, 'this->' is optional (unlike TypeScript's 'this.')

// 3. Using pev (pointer to entity vars):
//    - pev->health instead of this->health
//    - pev is a pointer, so we use -> not .

// 4. Switch on enum:
//    - switch(getCurrentTaskId()) works because Task is an enum
//    - Need 'default' case for completeness

// 5. String formatting with ctrl.msg():
//    - Uses printf-style format specifiers
//    - %s = string, %d = int, %.0f = float with 0 decimals

// ============================================================
// EXTENDED VERSION: JSON-like output
// ============================================================

void Bot::reportStatusJSON() const {
    ctrl.msg("{");
    ctrl.msg("  \"name\": \"%s\",", pev->netname.chars());
    ctrl.msg("  \"team\": \"%s\",", m_team == Team::Terrorist ? "T" : "CT");
    ctrl.msg("  \"health\": %.0f,", pev->health);
    ctrl.msg("  \"position\": [%.0f, %.0f, %.0f],",
             pev->origin.x, pev->origin.y, pev->origin.z);
    ctrl.msg("  \"currentNode\": %d,", m_currentNodeIndex);
    ctrl.msg("  \"goalNode\": %d,", m_chosenGoalIndex);
    ctrl.msg("  \"isAlive\": %s,", m_isAlive ? "true" : "false");
    ctrl.msg("  \"hasEnemy\": %s", m_enemy != nullptr ? "true" : "false");
    ctrl.msg("}");
}
