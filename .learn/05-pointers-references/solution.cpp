// Solution for Lesson 05: Pointers & References
//
// This exercise creates a safe enemy info printer.

// ============================================================
// STEP 1: Add declaration in inc/yapb.h
// ============================================================
// In the Bot class public methods section, add:

void printEnemyInfo();

// ============================================================
// STEP 2: Implement in src/combat.cpp
// ============================================================

void Bot::printEnemyInfo() {
    // Safety check: is the enemy pointer valid?
    if (game.isNullEntity(m_enemy)) {
        ctrl.msg("%s: No enemy targeted",
                 pev->netname.chars());
        return;
    }

    // Now safe to dereference m_enemy
    // m_enemy->v gives us the entity variables (entvars_t)
    entvars_t* enemy = &m_enemy->v;

    // Calculate distance to enemy
    float dist = (pev->origin - enemy->origin).length();

    // Print detailed info
    ctrl.msg("%s targeting %s - HP: %.0f, Dist: %.0f, Pos: (%.0f, %.0f, %.0f)",
             pev->netname.chars(),        // Our bot's name
             enemy->netname.chars(),      // Enemy's name
             enemy->health,               // Enemy's health
             dist,                        // Distance to enemy
             enemy->origin.x,             // Position X
             enemy->origin.y,             // Position Y
             enemy->origin.z);            // Position Z
}

// ============================================================
// ALTERNATIVE: More detailed version with weapon info
// ============================================================

void Bot::printEnemyInfoDetailed() {
    // Null check
    if (game.isNullEntity(m_enemy)) {
        ctrl.msg("%s: No current enemy", pev->netname.chars());
        return;
    }

    const entvars_t& ev = m_enemy->v;  // Reference for cleaner access

    // Check if enemy is alive
    if (ev.health <= 0 || (ev.deadflag != DEAD_NO)) {
        ctrl.msg("%s: Enemy %s is dead",
                 pev->netname.chars(),
                 ev.netname.chars());
        return;
    }

    // Calculate useful info
    float distance = (pev->origin - ev.origin).length();
    bool isInFOV = this->isInFOV(ev.origin) < 90.0f;  // Within 90 degree FOV
    bool isDucking = (ev.flags & FL_DUCKING) != 0;

    ctrl.msg("Enemy Info for %s:", pev->netname.chars());
    ctrl.msg("  Name: %s", ev.netname.chars());
    ctrl.msg("  Health: %.0f/%.0f", ev.health, ev.max_health);
    ctrl.msg("  Distance: %.0f units", distance);
    ctrl.msg("  In FOV: %s", isInFOV ? "Yes" : "No");
    ctrl.msg("  Ducking: %s", isDucking ? "Yes" : "No");
    ctrl.msg("  Position: (%.0f, %.0f, %.0f)",
             ev.origin.x, ev.origin.y, ev.origin.z);
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class Bot {
    private enemy: Entity | null = null;

    printEnemyInfo(): void {
        // Null check
        if (this.enemy === null || this.enemy.isInvalid()) {
            console.log(`${this.name}: No enemy targeted`);
            return;
        }

        // Safe to access enemy now
        const dist = this.position.distanceTo(this.enemy.position);

        console.log(
            `${this.name} targeting ${this.enemy.name} - ` +
            `HP: ${this.enemy.health}, ` +
            `Dist: ${dist.toFixed(0)}, ` +
            `Pos: (${this.enemy.position.x.toFixed(0)}, ` +
            `${this.enemy.position.y.toFixed(0)}, ` +
            `${this.enemy.position.z.toFixed(0)})`
        );
    }
}
*/

// ============================================================
// KEY POINTER CONCEPTS DEMONSTRATED
// ============================================================

// 1. Null checking before dereference:
//    if (game.isNullEntity(m_enemy))
//    This prevents crashes from dereferencing null.

// 2. Arrow operator (->):
//    m_enemy->v
//    Equivalent to (*m_enemy).v but cleaner.

// 3. Getting pointer to member:
//    entvars_t* enemy = &m_enemy->v;
//    We take address of the v member.

// 4. Reference for cleaner access:
//    const entvars_t& ev = m_enemy->v;
//    Now we can use ev.health instead of m_enemy->v.health.

// 5. Pointer vs Value in struct:
//    m_enemy is a pointer (edict_t*)
//    m_enemy->v is a value (entvars_t, embedded in edict_t)
//    m_enemy->v.origin is a value (Vector)

// ============================================================
// MEMORY SAFETY PATTERNS
// ============================================================

// Pattern 1: Early return on null
void safeFn1(edict_t* ent) {
    if (!ent) return;  // or: if (game.isNullEntity(ent)) return;
    // Safe to use ent here
}

// Pattern 2: Default value on null
float getHealth(edict_t* ent) {
    if (!ent) return 0.0f;
    return ent->v.health;
}

// Pattern 3: Optional action
void maybeAttack(edict_t* target) {
    if (target && target->v.health > 0) {
        // Attack only if target exists and is alive
    }
}

// Pattern 4: Using reference when null is not allowed
void mustHaveTarget(edict_t& target) {
    // target cannot be null - caller guarantees validity
    target.v.health -= 10;
}
