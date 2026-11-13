#include "filter.h"

/* k = dt / (RC + dt),  where RC = 1 / (2*pi*cutoff) */
void pt1_set_cutoff(pt1_t *f, fix16_t cutoff_hz, fix16_t dt) {
    fix16_t two_pi_fc = fix16_mul(F16(6.283185), cutoff_hz);
    fix16_t rc        = fix16_div(F16(1.0), two_pi_fc);
    f->k = fix16_div(dt, fix16_add(rc, dt));
}

void pt1_init(pt1_t *f, fix16_t cutoff_hz, fix16_t dt) {
    f->state = 0;
    pt1_set_cutoff(f, cutoff_hz, dt);
}

/* state += k * (input - state) */
fix16_t pt1_apply(pt1_t *f, fix16_t input) {
    fix16_t error = fix16_sub(input, f->state);
    f->state = fix16_add(f->state, fix16_mul(f->k, error));
    return f->state;
}
