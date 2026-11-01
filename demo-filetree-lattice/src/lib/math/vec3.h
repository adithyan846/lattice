#ifndef LIB_MATH_VEC3_H
#define LIB_MATH_VEC3_H

#ifdef __cplusplus
extern "C" {
#endif

/** Right-handed 3-vector. Units depend on the caller. */
typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

vec3_t vec3_make(float x, float y, float z);
vec3_t vec3_zero(void);

vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_scale(vec3_t v, float scalar);
vec3_t vec3_negate(vec3_t v);

float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_cross(vec3_t a, vec3_t b);
float vec3_norm_squared(vec3_t v);
float vec3_norm(vec3_t v);

/** Returns a unit vector, or the zero vector if the input is (near) zero. */
vec3_t vec3_normalize(vec3_t v);

/** Component-wise clamp to [min, max]. */
vec3_t vec3_clamp(vec3_t v, float min_value, float max_value);

/** Linear interpolation: a + (b - a) * t, t clamped to [0, 1]. */
vec3_t vec3_lerp(vec3_t a, vec3_t b, float t);

#ifdef __cplusplus
}
#endif

#endif /* LIB_MATH_VEC3_H */
