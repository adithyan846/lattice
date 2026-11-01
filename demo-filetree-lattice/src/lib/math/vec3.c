#include <lib/math/vec3.h>

#include <math.h>

vec3_t vec3_make(float x, float y, float z)
{
    vec3_t v = {x, y, z};
    return v;
}

vec3_t vec3_zero(void)
{
    return vec3_make(0.0f, 0.0f, 0.0f);
}

vec3_t vec3_add(vec3_t a, vec3_t b)
{
    return vec3_make(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3_t vec3_sub(vec3_t a, vec3_t b)
{
    return vec3_make(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3_t vec3_scale(vec3_t v, float scalar)
{
    return vec3_make(v.x * scalar, v.y * scalar, v.z * scalar);
}

vec3_t vec3_negate(vec3_t v)
{
    return vec3_scale(v, -1.0f);
}

float vec3_dot(vec3_t a, vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3_t vec3_cross(vec3_t a, vec3_t b)
{
    return vec3_make(a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x);
}

float vec3_norm_squared(vec3_t v)
{
    return vec3_dot(v, v);
}

float vec3_norm(vec3_t v)
{
    return sqrtf(vec3_norm_squared(v));
}

vec3_t vec3_normalize(vec3_t v)
{
    const float n = vec3_norm(v);

    if (n < 1e-9f) {
        return vec3_zero();
    }

    return vec3_scale(v, 1.0f / n);
}

vec3_t vec3_clamp(vec3_t v, float min_value, float max_value)
{
    if (v.x < min_value) {
        v.x = min_value;
    } else if (v.x > max_value) {
        v.x = max_value;
    }

    if (v.y < min_value) {
        v.y = min_value;
    } else if (v.y > max_value) {
        v.y = max_value;
    }

    if (v.z < min_value) {
        v.z = min_value;
    } else if (v.z > max_value) {
        v.z = max_value;
    }

    return v;
}

vec3_t vec3_lerp(vec3_t a, vec3_t b, float t)
{
    if (t <= 0.0f) {
        return a;
    }

    if (t >= 1.0f) {
        return b;
    }

    return vec3_add(a, vec3_scale(vec3_sub(b, a), t));
}
