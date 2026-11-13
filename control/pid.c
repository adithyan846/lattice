#include "pid.h"

void pid_init(pid_ctrl_t *pid, const pid_params_t *params, fix16_t dt) {
    pid->params           = *params;
    pid->integral         = 0;
    pid->prev_measurement = 0;
    pt1_init(&pid->dterm_filter, params->dterm_cutoff_hz, dt);
}

void pid_set_params(pid_ctrl_t *pid, const pid_params_t *params, fix16_t dt) {
    pid->params = *params;
    pt1_set_cutoff(&pid->dterm_filter, params->dterm_cutoff_hz, dt);
}

fix16_t pid_update(pid_ctrl_t *pid, fix16_t setpoint, fix16_t measurement, fix16_t dt) {
    const pid_params_t *p = &pid->params;
    fix16_t neg_limit = fix16_sub(0, p->out_limit);
    fix16_t error = fix16_sub(setpoint, measurement);

    /* P */
    fix16_t p_term = fix16_mul(p->kp, error);

    /* D on measurement, negated, then low-passed */
    fix16_t change     = fix16_sub(measurement, pid->prev_measurement);
    fix16_t derivative = fix16_div(change, dt);
    derivative = pt1_apply(&pid->dterm_filter, derivative);
    fix16_t d_term = fix16_mul(p->kd, fix16_sub(0, derivative));

    /* I: work out what the integral would become */
    fix16_t new_integral = fix16_add(pid->integral, fix16_mul(error, dt));
    if (new_integral >  p->i_limit) new_integral =  p->i_limit;
    if (new_integral < fix16_sub(0, p->i_limit)) new_integral = fix16_sub(0, p->i_limit);

    fix16_t i_term = fix16_mul(p->ki, new_integral);
    fix16_t output = fix16_add(fix16_add(p_term, i_term), d_term);

    /* conditional integration: if the output is clamped and the error would
     * push it further into the clamp, do not let the integral grow */
    int pushing_high = (output > p->out_limit) && (error > 0);
    int pushing_low  = (output < neg_limit)    && (error < 0);

    if (pushing_high || pushing_low) {
        i_term = fix16_mul(p->ki, pid->integral);      /* keep the old integral */
        output = fix16_add(fix16_add(p_term, i_term), d_term);
    } else {
        pid->integral = new_integral;
    }

    pid->prev_measurement = measurement;

    if (output >  p->out_limit) output = p->out_limit;
    if (output < neg_limit)     output = neg_limit;
    return output;
}
