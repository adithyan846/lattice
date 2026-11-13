#ifndef PID_H
#define PID_H

#include "fix16.h"
#include "filter.h"

typedef struct {
    fix16_t kp;
    fix16_t ki;
    fix16_t kd;
    fix16_t i_limit;
    fix16_t out_limit;        /* the largest output the actuator can use */
    fix16_t dterm_cutoff_hz;
} pid_params_t;

typedef struct {
    pid_params_t params;
    fix16_t integral;
    fix16_t prev_measurement;
    pt1_t   dterm_filter;
} pid_ctrl_t;

void    pid_init(pid_ctrl_t *pid, const pid_params_t *params, fix16_t dt);
void    pid_set_params(pid_ctrl_t *pid, const pid_params_t *params, fix16_t dt);
fix16_t pid_update(pid_ctrl_t *pid, fix16_t setpoint, fix16_t measurement, fix16_t dt);

#endif
