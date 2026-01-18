# Lesson 12: Vector Math & 3D Space

Master 3D vector operations essential for game AI - distance checks, direction detection, and aim calculations.

## Learning Objectives

- Understand 3D vectors and their operations
- Calculate distances between points
- Use dot product for direction detection
- Create a function to check if enemy is in front of bot

## Theory: What is a Vector?

A vector represents a point or direction in 3D space.

### TypeScript

```typescript
interface Vector {
    x: number;  // East/West
    y: number;  // North/South
    z: number;  // Up/Down
}

const position: Vector = { x: 100, y: 200, z: 50 };
const velocity: Vector = { x: 10, y: 0, z: -5 };  // Moving east, falling
```

### C++ (crlib)

```cpp
// File: ext/crlib/crlib/vector.h
template <typename T>
class Vec3D {
public:
    T x, y, z;

    // Constructors
    Vec3D(T scalar = 0) : x(scalar), y(scalar), z(scalar) {}
    Vec3D(T x, T y, T z) : x(x), y(y), z(z) {}

    // Operations defined below...
};

using Vector = Vec3D<float>;
```

## Theory: Vector Operations

### Addition/Subtraction

```cpp
Vector a(10, 20, 30);
Vector b(5, 5, 5);

Vector sum = a + b;   // (15, 25, 35)
Vector diff = a - b;  // (5, 15, 25)
```

Getting direction FROM a TO b:
```cpp
Vector direction = b - a;  // Vector pointing from a to b
```

### Scalar Multiplication/Division

```cpp
Vector v(10, 20, 30);

Vector doubled = v * 2;     // (20, 40, 60)
Vector halved = v / 2;      // (5, 10, 15)
Vector scaled = v * 0.5f;   // (5, 10, 15)
```

### Length (Magnitude)

```cpp
Vector v(3, 4, 0);
float len = v.length();  // 5 (3-4-5 triangle)

// How it works:
// length = sqrt(x*x + y*y + z*z)
// length = sqrt(9 + 16 + 0) = sqrt(25) = 5
```

### Distance Between Points

```cpp
Vector playerPos(100, 200, 50);
Vector enemyPos(150, 250, 50);

// Method 1: Calculate direction, then length
Vector toEnemy = enemyPos - playerPos;  // (50, 50, 0)
float distance = toEnemy.length();       // ~70.7

// Method 2: Direct distance function
float distance2 = (enemyPos - playerPos).length();

// Method 3: Squared distance (faster, no sqrt)
float distSq = playerPos.distanceSq(enemyPos);  // 5000
// Use when comparing distances: distSq < radiusSq
```

### Normalization

Make a vector length 1 (unit vector):

```cpp
Vector dir(10, 0, 0);
Vector normalized = dir.normalize();  // (1, 0, 0)

// Useful for directions without magnitude
Vector moveDir = (target - position).normalize();
Vector newPos = position + moveDir * speed * deltaTime;
```

## Theory: Dot Product

The **dot product** tells you how aligned two vectors are.

```cpp
// Operator | is dot product in crlib
float dot = vectorA | vectorB;

// Or explicitly:
float dot = vectorA.x * vectorB.x +
            vectorA.y * vectorB.y +
            vectorA.z * vectorB.z;
```

### Dot Product Values

```
dot > 0   →  Same general direction (in front)
dot = 0   →  Perpendicular (to the side)
dot < 0   →  Opposite directions (behind)
```

### Checking if Enemy is in Front

```cpp
bool isInFront(const Vector& myPos, const Vector& myForward, const Vector& enemyPos) {
    Vector toEnemy = (enemyPos - myPos).normalize();
    float dot = myForward | toEnemy;
    return dot > 0;  // Positive = in front
}
```

### Angle Between Vectors

```cpp
float angleBetween(const Vector& a, const Vector& b) {
    float dot = a.normalize() | b.normalize();
    return cr::acosf(dot) * (180.0f / cr::kFloatMathPi);  // Degrees
}
```

## Code Study: Vec3D Implementation

### File: `ext/crlib/crlib/vector.h:58-91`

```cpp
// Addition
constexpr Vec3D operator + (const Vec3D &rhs) const {
   return Vec3D { x + rhs.x, y + rhs.y, z + rhs.z };
}

// Subtraction
constexpr Vec3D operator - (const Vec3D &rhs) const {
   return Vec3D { x - rhs.x, y - rhs.y, z - rhs.z };
}

// Cross product (^)
constexpr Vec3D operator ^ (const Vec3D &rhs) const {
   return Vec3D { y * rhs.z - z * rhs.y,
                  z * rhs.x - x * rhs.z,
                  x * rhs.y - y * rhs.x };
}

// Dot product (|)
constexpr T operator | (const Vec3D &rhs) const {
   return x * rhs.x + y * rhs.y + z * rhs.z;
}
```

## Code Study: YaPB Combat Use

### File: `src/combat.cpp:28-36` (in numEnemiesNear)

```cpp
const float radiusSq = cr::sqrf(radius);

for (const auto &client : util.getClients()) {
    // ...
    if (client.origin.distanceSq(origin) < radiusSq) {
        count++;
    }
}
```

Uses squared distance for efficient comparisons (avoids sqrt).

## Exercise: Check if Enemy is in Front

Create a function that checks if an enemy is in front of the bot.

### Your Task

1. Get bot's forward direction from angles
2. Calculate direction to enemy
3. Use dot product to check alignment
4. Return whether enemy is in front

### Implementation

```cpp
bool Bot::isEnemyInFront(edict_t* enemy) {
    if (game.isNullEntity(enemy)) {
        return false;
    }

    // Get bot's forward direction from view angles
    Vector forward, right, up;
    pev->v_angle.angleVectors(&forward, &right, &up);

    // Direction from bot to enemy
    Vector toEnemy = (enemy->v.origin - pev->origin).normalize();

    // Dot product: positive = in front
    float dot = forward | toEnemy;

    // Check if within ~90 degrees (dot > 0)
    // Or narrower: dot > 0.5 for ~60 degrees
    return dot > 0.0f;
}
```

### Extended Version: With Angle Threshold

```cpp
bool Bot::isEnemyInFOV(edict_t* enemy, float maxAngle) {
    if (game.isNullEntity(enemy)) {
        return false;
    }

    Vector forward, right, up;
    pev->v_angle.angleVectors(&forward, &right, &up);

    Vector toEnemy = (enemy->v.origin - pev->origin).normalize();

    float dot = forward | toEnemy;

    // Convert dot product to angle threshold
    // cos(60°) ≈ 0.5, cos(45°) ≈ 0.707, cos(30°) ≈ 0.866
    float threshold = cr::cosf(maxAngle * cr::kFloatMathPi / 180.0f);

    return dot > threshold;
}

// Usage:
if (isEnemyInFOV(enemy, 45.0f)) {
    // Enemy within 45 degree cone
}
```

### Build and Test

Add debug output:
```cpp
bool Bot::isEnemyInFront(edict_t* enemy) {
    // ... calculations ...

    float dot = forward | toEnemy;
    float angle = cr::acosf(dot) * 180.0f / cr::kFloatMathPi;

    ctrl.msg("%s: Enemy at %.1f degrees (dot=%.2f)",
             pev->netname.chars(), angle, dot);

    return dot > 0.0f;
}
```

## Solution

See `solution.cpp` for the complete implementation.

## Deep Dive: Angles to Vectors

### Converting View Angles to Direction

```cpp
// pev->v_angle contains (pitch, yaw, roll) in degrees
Vector angles = pev->v_angle;

Vector forward, right, up;
angles.angleVectors(&forward, &right, &up);

// forward: where you're looking
// right: direction to your right
// up: direction above you
```

### YaPB Helper

```cpp
// File: ext/crlib/crlib/vector.h
void angleVectors(Vector* forward, Vector* right, Vector* up) const {
    // Converts pitch/yaw/roll to directional vectors
    // ... trigonometry implementation
}
```

## Key Concepts Learned

| Concept | Description |
|---------|-------------|
| Vector | 3D point or direction (x, y, z) |
| Length | Distance from origin: `sqrt(x²+y²+z²)` |
| Normalize | Make length 1, keep direction |
| Dot product | Alignment check: `a·b = |a||b|cos(θ)` |
| `distanceSq` | Squared distance (faster) |
| `angleVectors` | Convert angles to direction vectors |

## TypeScript to C++ Cheatsheet

```typescript
// TypeScript
const distance = Math.sqrt(
    Math.pow(a.x - b.x, 2) +
    Math.pow(a.y - b.y, 2) +
    Math.pow(a.z - b.z, 2)
);

// C++
float distance = (a - b).length();
// or
float distance = a.distance(b);
```

```typescript
// TypeScript dot product
const dot = a.x * b.x + a.y * b.y + a.z * b.z;

// C++
float dot = a | b;  // Operator overload
```

## What's Next

In Lesson 13, you'll learn about A* Pathfinding:

- Navigation graphs and nodes
- The A* algorithm
- How bots find paths

**[Next: Lesson 13 - Navigation & A* Pathfinding](../13-pathfinding/README.md)**
