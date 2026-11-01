#ifndef LIB_MATH_QUAT_H
#define LIB_MATH_QUAT_H

#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Unit quaternion, scalar-first: q = w + x*i + y*j + z*k. */
typedef struct {
    float w;
    float x;
    float y;
    float z;
} quat_t;

quat_t quat_identity(void);
quat_t quat_make(float w, float x, float y, float z);

/** ZYX Euler angles in radians (yaw, then pitch, then roll). */
quat_t quat_from_euler(float roll, float pitch, float yaw);
void quat_to_euler(quat_t q, float *roll, float *pitch, float *yaw);

quat_t quat_multiply(quat_t a, quat_t b);
quat_t quat_conjugate(quat_t q);
quat_t quat_normalize(quat_t q);
float quat_norm(quat_t q);

/** Rotate a vector from body to world frame (q maps body -> world). */
vec3_t quat_rotate(quat_t q, vec3_t v);

/** Error quaternion for a shortest-path rotation from `from` to `to`. */
quat_t quat_error(quat_t from, quat_t to);

#ifdef __cplusplus
}
#endif

#endif /* LIB_MATH_QUAT_H */
