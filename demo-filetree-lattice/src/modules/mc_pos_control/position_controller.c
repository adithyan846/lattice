#include <modules/mc_pos_control/position_controller.h>

#include <math.h>
#include <stddef.h>

#include <lib/math/quat.h>

#define GRAVITY_MPS2 9.80665f

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

static float clamp_length(float x, float y, float limit)
{
    const float length = sqrtf(x * x + y * y);

    if (length > limit) {
        return limit / length;
    }

    return 1.0f;
}

void position_controller_default_config(position_controller_config_t *config)
{
    if (config == NULL) {
        return;
    }

    config->kp_xy = 0.8f;
    config->kp_z = 1.0f;
    config->kp_vel_xy = 1.5f;
    config->kp_vel_z = 2.0f;
    config->max_velocity_xy_mps = 5.0f;
    config->max_velocity_z_mps = 1.5f;
    config->max_tilt_rad = 0.610865f; /* 35 degrees */
    config->hover_thrust = 0.5f;
}

void position_controller_init(position_controller_t *controller,
                              const position_controller_config_t *config)
{
    if (controller == NULL) {
        return;
    }

    if (config != NULL) {
        controller->config = *config;

    } else {
        position_controller_default_config(&controller->config);
    }
}

int position_controller_update(position_controller_t *controller, vec3_t position_setpoint,
                               vec3_t velocity_setpoint, vec3_t position, vec3_t velocity,
                               float yaw_setpoint, vec3_t *velocity_output, quat_t *attitude_setpoint,
                               float *thrust)
{
    if (controller == NULL || velocity_output == NULL || attitude_setpoint == NULL || thrust == NULL) {
        return -1;
    }

    const position_controller_config_t *config = &controller->config;

    /* Outer loop: position error adds to the velocity feed-forward. */
    float velocity_north = velocity_setpoint.x + config->kp_xy * (position_setpoint.x - position.x);
    float velocity_east = velocity_setpoint.y + config->kp_xy * (position_setpoint.y - position.y);
    float velocity_down = velocity_setpoint.z + config->kp_z * (position_setpoint.z - position.z);

    const float velocity_scale = clamp_length(velocity_north, velocity_east, config->max_velocity_xy_mps);
    velocity_north *= velocity_scale;
    velocity_east *= velocity_scale;
    velocity_down = clampf(velocity_down, -config->max_velocity_z_mps, config->max_velocity_z_mps);

    /* Inner loop: velocity error to acceleration. */
    float acceleration_north = config->kp_vel_xy * (velocity_north - velocity.x);
    float acceleration_east = config->kp_vel_xy * (velocity_east - velocity.y);
    const float acceleration_down = config->kp_vel_z * (velocity_down - velocity.z);

    /* Limit the horizontal acceleration so the attitude setpoint respects the
     * tilt limit. */
    const float max_horizontal_acceleration = GRAVITY_MPS2 * tanf(config->max_tilt_rad);
    const float acceleration_scale =
        clamp_length(acceleration_north, acceleration_east, max_horizontal_acceleration);
    acceleration_north *= acceleration_scale;
    acceleration_east *= acceleration_scale;

    /* Approximate attitude from the desired horizontal acceleration. Positive
     * roll produces acceleration to the right (east); positive pitch (nose up)
     * produces acceleration to the rear (south). Verify signs in SITL. */
    const float roll_setpoint = atan2f(acceleration_east, GRAVITY_MPS2);
    const float pitch_setpoint = -atan2f(acceleration_north, GRAVITY_MPS2);

    /* Vertical thrust feed-forward plus correction. The output uses the same
     * normalised scale as the flight modes (0.5 hovers). */
    *thrust = clampf(config->hover_thrust + acceleration_down / GRAVITY_MPS2, 0.0f, 1.0f);

    *attitude_setpoint = quat_from_euler(roll_setpoint, pitch_setpoint, yaw_setpoint);
    *velocity_output = vec3_make(velocity_north, velocity_east, velocity_down);

    return 0;
}
