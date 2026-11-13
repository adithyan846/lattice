#ifndef ANGLE_H
#define ANGLE_H

#include "fix16.h"

/* Outer loop of the cascade: turns an angle error into a rate setpoint.
 *
 *   angle setpoint --> [angle loop] --> rate setpoint --> [rate PID] --> torque
 *
 * It is a plain P controller with a rate limit. The rate PID underneath
 * already does the fast, precise work, so the outer loop only has to say
 * "how fast should we be rotating to close this angle gap".               */
typedef struct {
    fix16_t kp;          /* deg/s of rate per degree of angle error */
    fix16_t max_rate;    /* deg/s: never ask the rate loop for more than this */
} angle_ctrl_t;

void    angle_init(angle_ctrl_t *a, fix16_t kp, fix16_t max_rate);
fix16_t angle_update(const angle_ctrl_t *a, fix16_t angle_setpoint, fix16_t angle_measured);

#endif
