#include "test_assert.h"

#include <lib/math/pid.h>

void test_pid(void)
{
    pid_t pid;
    pid_init(&pid, 2.0f, 0.0f, 0.0f, -10.0f, 10.0f);

    /* Proportional only: error 1 -> output 2. */
    TEST_ASSERT_NEAR(pid_update(&pid, 1.0f, 0.0f, 0.01f), 2.0f, 1e-6);

    /* Output saturation. */
    pid_init(&pid, 10.0f, 0.0f, 0.0f, -5.0f, 5.0f);
    TEST_ASSERT_NEAR(pid_update(&pid, 1.0f, 0.0f, 0.01f), 5.0f, 1e-6);

    /* Integral action accumulates over time. */
    pid_init(&pid, 0.0f, 1.0f, 0.0f, -10.0f, 10.0f);
    for (int i = 0; i < 100; i++) {
        (void)pid_update(&pid, 1.0f, 0.0f, 0.01f);
    }
    TEST_ASSERT_NEAR(pid.integrator, 1.0f, 1e-4);

    /* Derivative on measurement: measurement step 1 over 1 s -> -1. */
    pid_init(&pid, 0.0f, 0.0f, 1.0f, -10.0f, 10.0f);
    (void)pid_update(&pid, 1.0f, 0.0f, 1.0f);
    TEST_ASSERT_NEAR(pid_update(&pid, 1.0f, 1.0f, 1.0f), -1.0f, 1e-6);

    /* Reset clears the integrator. */
    pid_init(&pid, 1.0f, 1.0f, 0.0f, -10.0f, 10.0f);
    (void)pid_update(&pid, 1.0f, 0.0f, 1.0f);
    pid_reset(&pid);
    TEST_ASSERT_NEAR(pid.integrator, 0.0f, 1e-9);

    /* Invalid dt must be harmless. */
    pid_init(&pid, 1.0f, 0.0f, 0.0f, -10.0f, 10.0f);
    TEST_ASSERT_NEAR(pid_update(&pid, 1.0f, 0.0f, 0.0f), 0.0f, 1e-9);
}
