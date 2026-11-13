#include "attitude.h"

void attitude_init(attitude_t *a, fix16_t alpha, fix16_t acc_cutoff_hz, fix16_t dt) {
    a->roll  = 0;
    a->pitch = 0;
    a->alpha = alpha;
    a->one_minus_alpha = fix16_sub(F16(1.0), alpha);
    a->acc_min_sq = F16(0.81);     /* 0.9 g, squared */
    a->acc_max_sq = F16(1.21);     /* 1.1 g, squared */
    for (int i = 0; i < 3; i++) pt1_init(&a->acc_lpf[i], acc_cutoff_hz, dt);
    a->acc_lpf_primed = 0;
    a->acc_used = 0;
    a->initialised = 0;
}

void attitude_update(attitude_t *a,
                     fix16_t gx, fix16_t gy,
                     fix16_t ax, fix16_t ay, fix16_t az,
                     fix16_t dt) {
    /* start the filters from the first real reading, not from 0 */
    if (!a->acc_lpf_primed) {
        a->acc_lpf[0].state = ax;
        a->acc_lpf[1].state = ay;
        a->acc_lpf[2].state = az;
        a->acc_lpf_primed = 1;
    }

    /* smooth the accelerometer before trusting it */
    ax = pt1_apply(&a->acc_lpf[0], ax);
    ay = pt1_apply(&a->acc_lpf[1], ay);
    az = pt1_apply(&a->acc_lpf[2], az);

    /* gyro prediction: where we were, plus how far we rotated */
    fix16_t roll_pred  = fix16_add(a->roll,  fix16_mul(gx, dt));
    fix16_t pitch_pred = fix16_add(a->pitch, fix16_mul(gy, dt));

    /* gravity alone is exactly 1 g; far from 1 g means the drone is accelerating */
    fix16_t mag_sq = fix16_add(fix16_add(fix16_mul(ax, ax), fix16_mul(ay, ay)),
                               fix16_mul(az, az));
    a->acc_used = (mag_sq >= a->acc_min_sq) && (mag_sq <= a->acc_max_sq);

    if (!a->acc_used) {
        a->roll  = roll_pred;
        a->pitch = pitch_pred;
        return;
    }

    /* tilt from the direction of gravity */
    fix16_t roll_acc  = fix16_atan2(ay, az);
    fix16_t yz        = fix16_sqrt(fix16_add(fix16_mul(ay, ay), fix16_mul(az, az)));
    fix16_t pitch_acc = fix16_atan2(fix16_sub(0, ax), yz);

    /* first good reading: start from the accelerometer instead of 0 */
    if (!a->initialised) {
        a->roll  = roll_acc;
        a->pitch = pitch_acc;
        a->initialised = 1;
        return;
    }

    /* angle = pred + (1 - alpha) * (acc - pred) */
    a->roll  = fix16_add(roll_pred,
                 fix16_mul(a->one_minus_alpha, fix16_sub(roll_acc, roll_pred)));
    a->pitch = fix16_add(pitch_pred,
                 fix16_mul(a->one_minus_alpha, fix16_sub(pitch_acc, pitch_pred)));
}
