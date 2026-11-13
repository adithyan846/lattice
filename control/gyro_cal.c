#include "gyro_cal.h"

void gyro_cal_init(gyro_cal_t *c, int32_t samples, fix16_t max_spread) {
    c->sum = 0;
    c->count = 0;
    c->target = samples;
    c->bias = 0;
    c->min = FIX16_MAX;
    c->max = FIX16_MIN;
    c->max_spread = max_spread;
    c->done = 0;
    c->failed = 0;
}

/* call once per loop while the aircraft is stationary */
void gyro_cal_feed(gyro_cal_t *c, fix16_t rate) {
    if (c->done) return;

    c->sum += rate;
    c->count++;
    if (rate < c->min) c->min = rate;
    if (rate > c->max) c->max = rate;

    if (c->count >= c->target) {
        /* if the readings moved around a lot, the aircraft was not still */
        if (fix16_sub(c->max, c->min) > c->max_spread) {
            c->failed = 1;
        } else {
            c->bias = (fix16_t)(c->sum / c->count);
        }
        c->done = 1;
    }
}

fix16_t gyro_cal_apply(const gyro_cal_t *c, fix16_t rate) {
    return fix16_sub(rate, c->bias);
}
