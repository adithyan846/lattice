#include "test_assert.h"

#include <lib/math/quat.h>
#include <lib/math/vec3.h>

#include <math.h>

void test_vec3(void)
{
    const vec3_t a = vec3_make(1.0f, 2.0f, 3.0f);
    const vec3_t b = vec3_make(4.0f, 5.0f, 6.0f);

    const vec3_t sum = vec3_add(a, b);
    TEST_ASSERT_NEAR(sum.x, 5.0f, 1e-6);
    TEST_ASSERT_NEAR(sum.y, 7.0f, 1e-6);
    TEST_ASSERT_NEAR(sum.z, 9.0f, 1e-6);

    const vec3_t difference = vec3_sub(b, a);
    TEST_ASSERT_NEAR(difference.x, 3.0f, 1e-6);
    TEST_ASSERT_NEAR(difference.y, 3.0f, 1e-6);
    TEST_ASSERT_NEAR(difference.z, 3.0f, 1e-6);

    TEST_ASSERT_NEAR(vec3_dot(a, b), 32.0f, 1e-6);

    const vec3_t cross = vec3_cross(vec3_make(1.0f, 0.0f, 0.0f),
                                    vec3_make(0.0f, 1.0f, 0.0f));
    TEST_ASSERT_NEAR(cross.x, 0.0f, 1e-6);
    TEST_ASSERT_NEAR(cross.y, 0.0f, 1e-6);
    TEST_ASSERT_NEAR(cross.z, 1.0f, 1e-6);

    TEST_ASSERT_NEAR(vec3_norm(vec3_make(3.0f, 4.0f, 0.0f)), 5.0f, 1e-6);

    const vec3_t normalized = vec3_normalize(vec3_make(10.0f, 0.0f, 0.0f));
    TEST_ASSERT_NEAR(normalized.x, 1.0f, 1e-6);
    TEST_ASSERT_NEAR(vec3_norm(vec3_normalize(vec3_make(1.0f, 2.0f, 3.0f))), 1.0f, 1e-6);

    const vec3_t zero = vec3_normalize(vec3_zero());
    TEST_ASSERT_NEAR(vec3_norm(zero), 0.0f, 1e-9);
}

void test_quat(void)
{
    const quat_t identity = quat_identity();
    const vec3_t v = vec3_make(1.0f, 2.0f, 3.0f);
    const vec3_t rotated_identity = quat_rotate(identity, v);
    TEST_ASSERT_NEAR(rotated_identity.x, v.x, 1e-6);
    TEST_ASSERT_NEAR(rotated_identity.y, v.y, 1e-6);
    TEST_ASSERT_NEAR(rotated_identity.z, v.z, 1e-6);

    /* 90 degrees of yaw maps body +X to world +Y. */
    const quat_t yaw_90 = quat_from_euler(0.0f, 0.0f, 1.57079632679f);
    const vec3_t rotated = quat_rotate(yaw_90, vec3_make(1.0f, 0.0f, 0.0f));
    TEST_ASSERT_NEAR(rotated.x, 0.0f, 1e-5);
    TEST_ASSERT_NEAR(rotated.y, 1.0f, 1e-5);
    TEST_ASSERT_NEAR(rotated.z, 0.0f, 1e-5);

    const quat_t q = quat_from_euler(0.1f, -0.2f, 0.3f);
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    quat_to_euler(q, &roll, &pitch, &yaw);
    TEST_ASSERT_NEAR(roll, 0.1f, 1e-5);
    TEST_ASSERT_NEAR(pitch, -0.2f, 1e-5);
    TEST_ASSERT_NEAR(yaw, 0.3f, 1e-5);

    const quat_t error = quat_error(q, q);
    TEST_ASSERT_NEAR(error.w, 1.0f, 1e-6);
    TEST_ASSERT_NEAR(error.x, 0.0f, 1e-6);
    TEST_ASSERT_NEAR(error.y, 0.0f, 1e-6);
    TEST_ASSERT_NEAR(error.z, 0.0f, 1e-6);
}
