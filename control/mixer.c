#include "mixer.h"

void mixer_init_quadx(mixer_t *m, fix16_t idle) {
    /* roll right  -> left motors faster  (roll  +1 on left, -1 on right)
     * nose up     -> front motors faster (pitch +1 on front, -1 on rear)
     * nose right  -> CCW motors faster   (yaw   +1 on CCW, -1 on CW):
     *   a CCW prop pushes the frame clockwise, i.e. nose right.          */
    /*                  roll        pitch       yaw            motor        */
    m->rows[0] = (mixer_row_t){ F16(-1.0), F16(-1.0), F16(-1.0) };  /* M1 rear right,  CW  */
    m->rows[1] = (mixer_row_t){ F16(-1.0), F16( 1.0), F16( 1.0) };  /* M2 front right, CCW */
    m->rows[2] = (mixer_row_t){ F16( 1.0), F16(-1.0), F16( 1.0) };  /* M3 rear left,   CCW */
    m->rows[3] = (mixer_row_t){ F16( 1.0), F16( 1.0), F16(-1.0) };  /* M4 front left,  CW  */
    m->idle = idle;
}

static fix16_t clamp01(fix16_t v) {
    if (v < 0) return 0;
    if (v > FIX16_ONE) return FIX16_ONE;
    return v;
}

int mixer_update(const mixer_t *m, fix16_t throttle,
                 fix16_t roll, fix16_t pitch, fix16_t yaw,
                 fix16_t out[MIXER_MOTORS]) {
    fix16_t diff[MIXER_MOTORS];
    fix16_t lo = FIX16_MAX, hi = FIX16_MIN;
    int squeezed = 0;

    /* 1. each motor's share of the roll/pitch/yaw correction */
    for (int i = 0; i < MIXER_MOTORS; i++) {
        const mixer_row_t *r = &m->rows[i];
        diff[i] = fix16_add(fix16_add(fix16_mul(r->roll, roll),
                                      fix16_mul(r->pitch, pitch)),
                            fix16_mul(r->yaw, yaw));
        if (diff[i] < lo) lo = diff[i];
        if (diff[i] > hi) hi = diff[i];
    }

    /* 2. the usable range is idle .. 1. If the corrections are wider than
     *    that, shrink them all by the same factor (keeps the balance). */
    fix16_t room = fix16_sub(FIX16_ONE, m->idle);
    fix16_t span = fix16_sub(hi, lo);
    if (span > room) {
        fix16_t scale = fix16_div(room, span);
        for (int i = 0; i < MIXER_MOTORS; i++) diff[i] = fix16_mul(diff[i], scale);
        lo = fix16_mul(lo, scale);
        hi = fix16_mul(hi, scale);
        squeezed = 1;
    }

    /* 3. shift throttle up or down so every motor fits between idle and 1.
     *    Stability wins over throttle: the corrections stay, height may not. */
    fix16_t t = throttle;
    if (fix16_add(t, lo) < m->idle)     { t = fix16_sub(m->idle, lo);     squeezed = 1; }
    if (fix16_add(t, hi) > FIX16_ONE)   { t = fix16_sub(FIX16_ONE, hi);   squeezed = 1; }

    for (int i = 0; i < MIXER_MOTORS; i++)
        out[i] = clamp01(fix16_add(t, diff[i]));

    return squeezed;
}

int32_t mixer_to_us(fix16_t out) {
    /* 1000 us = stop, 2000 us = full */
    return 1000 + (int32_t)(((int64_t)clamp01(out) * 1000 + 32768) >> 16);
}
