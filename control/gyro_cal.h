#ifndef GYRO_CAL_H
#define GYRO_CAL_H

#include "fix16.h"

typedef struct {
    int64_t sum;           /* running total of readings */
    int32_t count;         /* readings collected so far */
    int32_t target;        /* readings needed */
    fix16_t bias;          /* result, valid once done */
    fix16_t min, max;      /* range seen: detects movement */
    fix16_t max_spread;    /* reject if the range exceeds this */
    int     done;
    int     failed;
} gyro_cal_t;

void    gyro_cal_init(gyro_cal_t *c, int32_t samples, fix16_t max_spread);
void    gyro_cal_feed(gyro_cal_t *c, fix16_t rate);
fix16_t gyro_cal_apply(const gyro_cal_t *c, fix16_t rate);

#endif
