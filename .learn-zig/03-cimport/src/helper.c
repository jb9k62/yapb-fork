// Lesson 03 provided C helper.

#include "helper.h"

// Newton's method square root — enough precision for a float, no libm needed.
static float my_sqrt(float x) {
    if (x <= 0) return 0;
    float g = x;
    for (int i = 0; i < 6; i++) g = 0.5f * (g + x / g);
    return g;
}

float vec_len(Vec2 v) {
    return my_sqrt(v.x * v.x + v.y * v.y);
}
