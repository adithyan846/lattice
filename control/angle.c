#include "angle.h"

void angle_init(angle_ctrl_t *a, fix16_t kp, fix16_t max_rate) {
    a->kp = kp;
    a->max_rate = max_rate;
}

/* returns the rate setpoint in deg/s for the rate PID */
fix16_t angle_update(const angle_ctrl_t *a, fix16_t angle_setpoint, fix16_t angle_measured) {
    fix16_t error = fix16_sub(angle_setpoint, angle_measured);
    fix16_t rate_sp = fix16_mul(a->kp, error);

    if (rate_sp >  a->max_rate) rate_sp = a->max_rate;
    if (rate_sp < fix16_sub(0, a->max_rate)) rate_sp = fix16_sub(0, a->max_rate);
    return rate_sp;
}
