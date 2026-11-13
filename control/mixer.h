#ifndef MIXER_H
#define MIXER_H

#include "fix16.h"

/* Quad-X motor mixer.
 *
 * Turns four commands into four motor outputs:
 *   throttle  0 .. 1    (how hard all motors push together)
 *   roll     -1 .. +1   (positive = roll right)
 *   pitch    -1 .. +1   (positive = nose up)
 *   yaw      -1 .. +1   (positive = nose right)
 *
 * Outputs are 0 .. 1 per motor, then converted to ESC pulse widths.
 *
 * Motor order and directions (top view, nose up):
 *
 *        front
 *     M4(CW)   M2(CCW)
 *         \     /
 *          \   /
 *          /   \
 *         /     \
 *     M3(CCW)  M1(CW)
 *        rear
 *
 * Motor positions and spin directions follow the Betaflight "Quad X" layout.
 * Sign convention here: + pitch = nose up (check it matches your PID/IMU).
 * YOUR FRAME MUST MATCH THIS:
 * check motor positions and spin directions, props OFF, before flying.  */

#define MIXER_MOTORS 4

typedef struct {
    fix16_t roll, pitch, yaw;      /* weight of each axis for one motor */
} mixer_row_t;

typedef struct {
    mixer_row_t rows[MIXER_MOTORS];
    fix16_t idle;                  /* lowest output while armed, e.g. 0.05 */
} mixer_t;

void mixer_init_quadx(mixer_t *m, fix16_t idle);

/* out[i] in 0..1. Returns 1 if the mixer had to squeeze the outputs to fit. */
int  mixer_update(const mixer_t *m, fix16_t throttle,
                  fix16_t roll, fix16_t pitch, fix16_t yaw,
                  fix16_t out[MIXER_MOTORS]);

/* 0..1 -> ESC pulse width in microseconds, 1000..2000 */
int32_t mixer_to_us(fix16_t out);

#endif
