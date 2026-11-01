#include <modules/mc_att_control/attitude_controller.h>

#include <math.h>
#include <stddef.h>

#include <lib/math/quat.h>

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

void attitude_controller_default_config(attitude_controller_config_t *config)
{
    if (config == NULL) {
        return;
    }

    config->kp_roll = 6.0f;
    config->kp_pitch = 6.0f;
    config->kp_yaw = 2.5f;
    config->max_roll_rate = 3.0f;
    config->max_pitch_rate = 3.0f;
    config->max_yaw_rate = 1.5f;
}

void attitude_controller_init(attitude_controller_t *controller,
                              const attitude_controller_config_t *config)
{
    if (controller == NULL) {
        return;
    }

    if (config != NULL) {
        controller->config = *config;

    } else {
        attitude_controller_default_config(&controller->config);
    }

    controller->attitude_setpoint = quat_identity();
}

int attitude_controller_update(attitude_controller_t *controller, quat_t attitude,
                               quat_t attitude_setpoint, float yaw_rate_ff, vec3_t *rate_setpoint)
{
    if (controller == NULL || rate_setpoint == NULL) {
        return -1;
    }

    attitude = quat_normalize(attitude);
    attitude_setpoint = quat_normalize(attitude_setpoint);

    /* q_error is the rotation that takes the current attitude to the setpoint,
     * expressed in the body frame. For small angles its vector part is half the
     * rotation vector. */
    const quat_t error = quat_error(attitude, attitude_setpoint);

    const float kp_roll = controller->config.kp_roll;
    const float kp_pitch = controller->config.kp_pitch;
    const float kp_yaw = controller->config.kp_yaw;

    float roll_rate = 2.0f * kp_roll * error.x;
    float pitch_rate = 2.0f * kp_pitch * error.y;
    float yaw_rate = 2.0f * kp_yaw * error.z + yaw_rate_ff;

    roll_rate = clampf(roll_rate, -controller->config.max_roll_rate, controller->config.max_roll_rate);
    pitch_rate =
        clampf(pitch_rate, -controller->config.max_pitch_rate, controller->config.max_pitch_rate);
    yaw_rate = clampf(yaw_rate, -controller->config.max_yaw_rate, controller->config.max_yaw_rate);

    controller->attitude_setpoint = attitude_setpoint;
    *rate_setpoint = vec3_make(roll_rate, pitch_rate, yaw_rate);
    return 0;
}
