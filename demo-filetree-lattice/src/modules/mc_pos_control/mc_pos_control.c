#include <modules/mc_pos_control/mc_pos_control.h>

#include <stddef.h>

#include <lib/math/quat.h>
#include <middleware/log/log.h>

void mc_pos_control_init(mc_pos_control_t *control)
{
    if (control == NULL) {
        return;
    }

    position_controller_init(&control->controller, NULL);
}

int mc_pos_control_step(mc_pos_control_t *control, const vehicle_local_position_s *local_position,
                        const trajectory_setpoint_s *trajectory_setpoint,
                        vehicle_attitude_setpoint_s *attitude_setpoint)
{
    if (control == NULL || local_position == NULL || trajectory_setpoint == NULL ||
        attitude_setpoint == NULL) {
        return -1;
    }

    if (!local_position->xy_valid || !local_position->z_valid) {
        return -1;
    }

    const vec3_t position = vec3_make(local_position->x, local_position->y, local_position->z);
    const vec3_t velocity = vec3_make(local_position->vx, local_position->vy, local_position->vz);
    const vec3_t position_setpoint = vec3_make(trajectory_setpoint->position[0],
                                               trajectory_setpoint->position[1],
                                               trajectory_setpoint->position[2]);
    const vec3_t velocity_setpoint = vec3_make(trajectory_setpoint->velocity[0],
                                               trajectory_setpoint->velocity[1],
                                               trajectory_setpoint->velocity[2]);

    vec3_t velocity_output;
    quat_t attitude;
    float thrust;

    if (position_controller_update(&control->controller, position_setpoint, velocity_setpoint,
                                   position, velocity, trajectory_setpoint->yaw, &velocity_output,
                                   &attitude, &thrust) != 0) {
        return -1;
    }

    attitude_setpoint->timestamp = trajectory_setpoint->timestamp;
    attitude_setpoint->q_d[0] = attitude.w;
    attitude_setpoint->q_d[1] = attitude.x;
    attitude_setpoint->q_d[2] = attitude.y;
    attitude_setpoint->q_d[3] = attitude.z;
    attitude_setpoint->thrust = thrust;

    return 0;
}

int mc_pos_control_start(void)
{
    /* TODO: subscribe to vehicle_local_position and trajectory_setpoint, run
     * the position loop at 100 Hz and publish vehicle_attitude_setpoint. */
    FRT_LOG_WARN("mc_pos_control: scaffold only, position task not implemented");
    return -1;
}

int mc_pos_control_stop(void)
{
    return 0;
}
