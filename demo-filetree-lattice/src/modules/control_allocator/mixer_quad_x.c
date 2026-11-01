#include <modules/control_allocator/mixer_quad_x.h>

#include <stddef.h>

#include <lib/math/vec3.h>

static float clampf(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

void mixer_quad_x_default_config(mixer_quad_x_config_t *config)
{
    if (config == NULL) {
        return;
    }

    config->torque_gain = 2.0f;
    config->motor_min = 0.0f;
    config->motor_max = 1.0f;
}

int mixer_quad_x_mix(const mixer_quad_x_config_t *config, vec3_t torque, float thrust,
                     float motors[MIXER_QUAD_X_MOTOR_COUNT])
{
    if (config == NULL || motors == NULL) {
        return -1;
    }

    const float roll = torque.x * config->torque_gain;
    const float pitch = torque.y * config->torque_gain;
    const float yaw = torque.z * config->torque_gain;

    /* Mixer matrix. Each column is [roll, pitch, yaw] for one motor. */
    const float mix[MIXER_QUAD_X_MOTOR_COUNT][3] = {
        {-1.0f, 1.0f, 1.0f},  /* front-right */
        {1.0f, -1.0f, 1.0f},  /* rear-left */
        {1.0f, 1.0f, -1.0f},  /* front-left */
        {-1.0f, -1.0f, -1.0f} /* rear-right */
    };

    for (int motor = 0; motor < MIXER_QUAD_X_MOTOR_COUNT; motor++) {
        const float command =
            thrust + roll * mix[motor][0] + pitch * mix[motor][1] + yaw * mix[motor][2];
        motors[motor] = clampf(command, config->motor_min, config->motor_max);
    }

    return 0;
}
