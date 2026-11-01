#include <modules/mc_att_control/mc_att_control.h>

#include <stddef.h>

#include <lib/math/quat.h>
#include <middleware/log/log.h>

void mc_att_control_init(mc_att_control_t *control)
{
    if (control == NULL) {
        return;
    }

    attitude_controller_init(&control->attitude, NULL);
    rate_controller_init(&control->rate, NULL);
}

int mc_att_control_step(mc_att_control_t *control, const vehicle_attitude_s *attitude,
                        const vehicle_attitude_setpoint_s *attitude_setpoint,
                        const vehicle_angular_velocity_s *angular_velocity, float dt_s,
                        vehicle_rates_setpoint_s *rates_setpoint,
                        vehicle_torque_setpoint_s *torque_setpoint,
                        vehicle_thrust_setpoint_s *thrust_setpoint)
{
    if (control == NULL || attitude == NULL || attitude_setpoint == NULL ||
        angular_velocity == NULL || rates_setpoint == NULL || torque_setpoint == NULL ||
        thrust_setpoint == NULL || dt_s <= 0.0f) {
        return -1;
    }

    const quat_t q = quat_make(attitude->q[0], attitude->q[1], attitude->q[2], attitude->q[3]);
    const quat_t q_d = quat_make(attitude_setpoint->q_d[0], attitude_setpoint->q_d[1],
                                 attitude_setpoint->q_d[2], attitude_setpoint->q_d[3]);
    const vec3_t body_rates =
        vec3_make(angular_velocity->xyz[0], angular_velocity->xyz[1], angular_velocity->xyz[2]);

    vec3_t rate_setpoint;

    if (attitude_controller_update(&control->attitude, q, q_d, 0.0f, &rate_setpoint) != 0) {
        return -1;
    }

    vec3_t torque;

    if (rate_controller_update(&control->rate, rate_setpoint, body_rates, dt_s, &torque) != 0) {
        return -1;
    }

    rates_setpoint->timestamp = attitude_setpoint->timestamp;
    rates_setpoint->roll = rate_setpoint.x;
    rates_setpoint->pitch = rate_setpoint.y;
    rates_setpoint->yaw = rate_setpoint.z;
    rates_setpoint->thrust = attitude_setpoint->thrust;

    torque_setpoint->timestamp = attitude_setpoint->timestamp;
    torque_setpoint->xyz[0] = torque.x;
    torque_setpoint->xyz[1] = torque.y;
    torque_setpoint->xyz[2] = torque.z;

    /* A multicopter has a single collective thrust axis: body z. */
    thrust_setpoint->timestamp = attitude_setpoint->timestamp;
    thrust_setpoint->xyz[0] = 0.0f;
    thrust_setpoint->xyz[1] = 0.0f;
    thrust_setpoint->xyz[2] = attitude_setpoint->thrust;

    return 0;
}

int mc_att_control_start(void)
{
    /* TODO: subscribe to vehicle_attitude, vehicle_attitude_setpoint and
     * vehicle_angular_velocity; run the 400 Hz attitude / 1 kHz rate loop;
     * publish vehicle_rates_setpoint, vehicle_torque_setpoint and
     * vehicle_thrust_setpoint. */
    FRT_LOG_WARN("mc_att_control: scaffold only, control task not implemented");
    return -1;
}

int mc_att_control_stop(void)
{
    return 0;
}
