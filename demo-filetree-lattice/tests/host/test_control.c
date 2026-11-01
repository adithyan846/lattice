#include <modules/control_allocator/actuator_limits.h>
#include <modules/control_allocator/mixer_quad_x.h>
#include <modules/mc_att_control/attitude_controller.h>
#include <modules/mc_att_control/rate_controller.h>
#include <modules/mc_pos_control/position_controller.h>

#include <lib/math/quat.h>
#include <lib/math/vec3.h>

#include "test_assert.h"

void test_attitude_controller(void)
{
    attitude_controller_t controller;
    attitude_controller_init(&controller, NULL);

    vec3_t rates;
    TEST_ASSERT(attitude_controller_update(&controller, quat_identity(), quat_identity(), 0.0f,
                                           &rates) == 0);
    TEST_ASSERT_NEAR(rates.x, 0.0f, 1e-5);
    TEST_ASSERT_NEAR(rates.y, 0.0f, 1e-5);
    TEST_ASSERT_NEAR(rates.z, 0.0f, 1e-5);

    const quat_t rolled = quat_from_euler(0.2f, 0.0f, 0.0f);
    TEST_ASSERT(attitude_controller_update(&controller, rolled, quat_identity(), 0.0f, &rates) == 0);
    TEST_ASSERT(rates.x < 0.0f);
}

void test_rate_controller(void)
{
    rate_controller_t controller;
    rate_controller_init(&controller, NULL);

    vec3_t torque;
    TEST_ASSERT(rate_controller_update(&controller, vec3_make(1.0f, 0.0f, 0.0f),
                                       vec3_zero(), 0.01f, &torque) == 0);
    TEST_ASSERT(torque.x > 0.0f);
    TEST_ASSERT_NEAR(torque.y, 0.0f, 1e-5);
}

void test_position_controller(void)
{
    position_controller_t controller;
    position_controller_init(&controller, NULL);

    vec3_t velocity_output;
    quat_t attitude;
    float thrust;

    TEST_ASSERT(position_controller_update(&controller, vec3_make(0.0f, 0.0f, -10.0f),
                                           vec3_zero(), vec3_make(0.0f, 0.0f, -10.0f),
                                           vec3_zero(), 0.0f, &velocity_output, &attitude,
                                           &thrust) == 0);
    TEST_ASSERT_NEAR(thrust, 0.5f, 1e-5);
    TEST_ASSERT_NEAR(attitude.w, 1.0f, 1e-5);

    TEST_ASSERT(position_controller_update(&controller, vec3_make(10.0f, 0.0f, -10.0f),
                                           vec3_zero(), vec3_make(0.0f, 0.0f, -10.0f),
                                           vec3_zero(), 0.0f, &velocity_output, &attitude,
                                           &thrust) == 0);
    TEST_ASSERT(velocity_output.x > 0.0f);
}

void test_mixer(void)
{
    mixer_quad_x_config_t config;
    mixer_quad_x_default_config(&config);

    float motors[MIXER_QUAD_X_MOTOR_COUNT];
    TEST_ASSERT(mixer_quad_x_mix(&config, vec3_zero(), 0.5f, motors) == 0);

    for (int motor = 0; motor < MIXER_QUAD_X_MOTOR_COUNT; motor++) {
        TEST_ASSERT_NEAR(motors[motor], 0.5f, 1e-5);
    }

    TEST_ASSERT(mixer_quad_x_mix(&config, vec3_make(1.0f, 0.0f, 0.0f), 0.5f, motors) == 0);
    TEST_ASSERT(motors[0] < 0.5f); /* front-right */
    TEST_ASSERT(motors[1] > 0.5f); /* rear-left */
    TEST_ASSERT(motors[2] > 0.5f); /* front-left */
    TEST_ASSERT(motors[3] < 0.5f); /* rear-right */
}

void test_actuator_limits(void)
{
    actuator_limits_t limits;
    actuator_limits_init(&limits, 4, 0.0f, 1.0f, 1.0f);

    float commands[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    TEST_ASSERT(actuator_limits_apply(&limits, commands, 4, 0.1f) == 0);

    for (int motor = 0; motor < 4; motor++) {
        TEST_ASSERT_NEAR(commands[motor], 0.1f, 1e-5);
    }
}
