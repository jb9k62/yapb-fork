// Solution for Lesson 12: Vector Math & 3D Space
//
// Functions for checking enemy direction and distance.

// ============================================================
// BASIC: Is enemy in front?
// ============================================================

bool Bot::isEnemyInFront(edict_t* enemy) {
    if (game.isNullEntity(enemy)) {
        return false;
    }

    // Get bot's forward direction from view angles
    Vector forward, right, up;
    pev->v_angle.angleVectors(&forward, &right, &up);

    // Direction from bot to enemy (normalized)
    Vector toEnemy = (enemy->v.origin - pev->origin).normalize();

    // Dot product: how aligned are we?
    // 1.0 = perfectly aligned (looking directly at)
    // 0.0 = perpendicular (90 degrees to side)
    // -1.0 = opposite (looking away)
    float dot = forward | toEnemy;

    return dot > 0.0f;  // Positive = in front (within 90 degrees)
}

// ============================================================
// ADVANCED: Is enemy within FOV angle?
// ============================================================

bool Bot::isEnemyInFOV(edict_t* enemy, float maxAngleDegrees) {
    if (game.isNullEntity(enemy)) {
        return false;
    }

    Vector forward, right, up;
    pev->v_angle.angleVectors(&forward, &right, &up);

    Vector toEnemy = (enemy->v.origin - pev->origin).normalize();

    float dot = forward | toEnemy;

    // Convert desired angle to dot product threshold
    // cos(angle) gives us the dot product for that angle
    float radians = maxAngleDegrees * (cr::kFloatMathPi / 180.0f);
    float threshold = cr::cosf(radians);

    return dot >= threshold;
}

// Usage:
// isEnemyInFOV(enemy, 45.0f)  // Within 45 degree cone
// isEnemyInFOV(enemy, 90.0f)  // Within 90 degree half-sphere (in front)
// isEnemyInFOV(enemy, 180.0f) // Anywhere (always true)

// ============================================================
// UTILITY: Get angle to enemy
// ============================================================

float Bot::getAngleToEnemy(edict_t* enemy) {
    if (game.isNullEntity(enemy)) {
        return 180.0f;  // Not visible = behind
    }

    Vector forward, right, up;
    pev->v_angle.angleVectors(&forward, &right, &up);

    Vector toEnemy = (enemy->v.origin - pev->origin).normalize();

    float dot = forward | toEnemy;

    // Clamp dot to valid range for acos
    dot = cr::clamp(dot, -1.0f, 1.0f);

    // Convert to degrees
    float radians = cr::acosf(dot);
    float degrees = radians * (180.0f / cr::kFloatMathPi);

    return degrees;
}

// ============================================================
// UTILITY: Is enemy to the left or right?
// ============================================================

int Bot::getEnemySide(edict_t* enemy) {
    if (game.isNullEntity(enemy)) {
        return 0;  // Unknown
    }

    Vector forward, right, up;
    pev->v_angle.angleVectors(&forward, &right, &up);

    Vector toEnemy = (enemy->v.origin - pev->origin).normalize();

    // Dot with right vector
    float dotRight = right | toEnemy;

    if (dotRight > 0.3f) return 1;   // Right
    if (dotRight < -0.3f) return -1; // Left
    return 0;  // Ahead or behind
}

// ============================================================
// DEBUG: Print direction info
// ============================================================

void Bot::debugEnemyDirection(edict_t* enemy) {
    if (game.isNullEntity(enemy)) {
        ctrl.msg("%s: No enemy", pev->netname.chars());
        return;
    }

    float angle = getAngleToEnemy(enemy);
    float distance = (enemy->v.origin - pev->origin).length();
    int side = getEnemySide(enemy);

    const char* sideStr = "CENTER";
    if (side > 0) sideStr = "RIGHT";
    if (side < 0) sideStr = "LEFT";

    ctrl.msg("%s: Enemy '%s' at %.1f degrees (%s), %.0f units away",
             pev->netname.chars(),
             enemy->v.netname.chars(),
             angle,
             sideStr,
             distance);
}

// ============================================================
// TYPESCRIPT EQUIVALENT
// ============================================================

/*
class Bot {
    isEnemyInFront(enemy: Entity | null): boolean {
        if (!enemy) return false;

        const forward = this.viewAngles.toForward();
        const toEnemy = enemy.position.subtract(this.position).normalize();

        const dot = forward.dot(toEnemy);
        return dot > 0;
    }

    isEnemyInFOV(enemy: Entity | null, maxAngle: number): boolean {
        if (!enemy) return false;

        const forward = this.viewAngles.toForward();
        const toEnemy = enemy.position.subtract(this.position).normalize();

        const dot = forward.dot(toEnemy);
        const threshold = Math.cos(maxAngle * Math.PI / 180);

        return dot >= threshold;
    }

    getAngleToEnemy(enemy: Entity | null): number {
        if (!enemy) return 180;

        const forward = this.viewAngles.toForward();
        const toEnemy = enemy.position.subtract(this.position).normalize();

        const dot = Math.max(-1, Math.min(1, forward.dot(toEnemy)));
        return Math.acos(dot) * 180 / Math.PI;
    }
}
*/

// ============================================================
// COMMON VECTOR OPERATIONS REFERENCE
// ============================================================

void vectorOperationsReference() {
    Vector a(10, 20, 30);
    Vector b(5, 5, 5);

    // Basic operations
    Vector add = a + b;        // (15, 25, 35)
    Vector sub = a - b;        // (5, 15, 25)
    Vector scaled = a * 2;     // (20, 40, 60)
    Vector divided = a / 2;    // (5, 10, 15)

    // Length
    float len = a.length();           // sqrt(10^2 + 20^2 + 30^2)
    float lenSq = a.lengthSq();       // 10^2 + 20^2 + 30^2 (no sqrt)

    // Distance
    float dist = a.distance(b);       // (a - b).length()
    float distSq = a.distanceSq(b);   // (a - b).lengthSq()

    // Normalization
    Vector norm = a.normalize();      // Length becomes 1

    // Dot product (angle alignment)
    float dot = a | b;                // a.x*b.x + a.y*b.y + a.z*b.z

    // Cross product (perpendicular vector)
    Vector cross = a ^ b;

    // Angle to vectors
    Vector forward, right, up;
    Vector angles(0, 90, 0);  // Looking east
    angles.angleVectors(&forward, &right, &up);
}

// ============================================================
// FOV ANGLES QUICK REFERENCE
// ============================================================

// Dot product thresholds for common angles:
// 90°  → cos(90°)  = 0.000
// 75°  → cos(75°)  = 0.259
// 60°  → cos(60°)  = 0.500
// 45°  → cos(45°)  = 0.707
// 30°  → cos(30°)  = 0.866
// 15°  → cos(15°)  = 0.966
// 0°   → cos(0°)   = 1.000

// Bot default FOV is usually 90-100 degrees
// Human peripheral vision is about 180 degrees
// Focused vision is about 60 degrees
