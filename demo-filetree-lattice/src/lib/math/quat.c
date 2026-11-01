#include <lib/math/quat.h>

#include <math.h>
#include <stddef.h>

quat_t quat_identity(void)
{
    return quat_make(1.0f, 0.0f, 0.0f, 0.0f);
}

quat_t quat_make(float w, float x, float y, float z)
{
    quat_t q = {w, x, y, z};
    return q;
}

quat_t quat_from_euler(float roll, float pitch, float yaw)
{
    const float cr = cosf(roll * 0.5f);
    const float sr = sinf(roll * 0.5f);
    const float cp = cosf(pitch * 0.5f);
    const float sp = sinf(pitch * 0.5f);
    const float cy = cosf(yaw * 0.5f);
    const float sy = sinf(yaw * 0.5f);

    return quat_make(cr * cp * cy + sr * sp * sy,
                     sr * cp * cy - cr * sp * sy,
                     cr * sp * cy + sr * cp * sy,
                     cr * cp * sy - sr * sp * cy);
}

void quat_to_euler(quat_t q, float *roll, float *pitch, float *yaw)
{
    q = quat_normalize(q);

    const float sin_pitch = 2.0f * (q.w * q.y - q.z * q.x);
    const float clamped = sin_pitch > 1.0f ? 1.0f : (sin_pitch < -1.0f ? -1.0f : sin_pitch);

    if (roll != NULL) {
        *roll = atan2f(2.0f * (q.w * q.x + q.y * q.z),
                       1.0f - 2.0f * (q.x * q.x + q.y * q.y));
    }

    if (pitch != NULL) {
        *pitch = asinf(clamped);
    }

    if (yaw != NULL) {
        *yaw = atan2f(2.0f * (q.w * q.z + q.x * q.y),
                      1.0f - 2.0f * (q.y * q.y + q.z * q.z));
    }
}

quat_t quat_multiply(quat_t a, quat_t b)
{
    return quat_make(a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
                     a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
                     a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
                     a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w);
}

quat_t quat_conjugate(quat_t q)
{
    return quat_make(q.w, -q.x, -q.y, -q.z);
}

float quat_norm(quat_t q)
{
    return sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

quat_t quat_normalize(quat_t q)
{
    const float n = quat_norm(q);

    if (n < 1e-9f) {
        return quat_identity();
    }

    const float inv = 1.0f / n;
    return quat_make(q.w * inv, q.x * inv, q.y * inv, q.z * inv);
}

vec3_t quat_rotate(quat_t q, vec3_t v)
{
    const quat_t qv = quat_make(0.0f, v.x, v.y, v.z);
    const quat_t rotated = quat_multiply(quat_multiply(q, qv), quat_conjugate(q));
    return vec3_make(rotated.x, rotated.y, rotated.z);
}

quat_t quat_error(quat_t from, quat_t to)
{
    quat_t error = quat_multiply(quat_conjugate(from), to);

    if (error.w < 0.0f) {
        error.w = -error.w;
        error.x = -error.x;
        error.y = -error.y;
        error.z = -error.z;
    }

    return error;
}
