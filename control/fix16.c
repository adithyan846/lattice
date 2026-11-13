#include "fix16.h"
#include "sin_table.h"
fix16_t fix16_add(fix16_t a, fix16_t b) {
    int64_t sum = (int64_t)a + (int64_t)b;
    if (sum > FIX16_MAX) return FIX16_MAX;
    if (sum < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)sum;
}

fix16_t fix16_sub(fix16_t a, fix16_t b) {
    int64_t diff = (int64_t)a - (int64_t)b;
    if (diff > FIX16_MAX) return FIX16_MAX;
    if (diff < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)diff;
}
fix16_t fix16_mul(fix16_t a, fix16_t b) {
    int64_t product = (int64_t)a * (int64_t)b;

    /* round to nearest instead of truncating toward zero */
    product += 32768;

    /* remove the second scale factor */
    product >>= 16;

    if (product > FIX16_MAX) return FIX16_MAX;
    if (product < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)product;
}
fix16_t fix16_div(fix16_t a, fix16_t b) {
    /* never divide by zero on a flying aircraft */
    if (b == 0) {
        return (a >= 0) ? FIX16_MAX : FIX16_MIN;
    }

    /* restore the scale factor that division cancels out */
    int64_t result = ((int64_t)a << 16) / b;

    if (result > FIX16_MAX) return FIX16_MAX;
    if (result < FIX16_MIN) return FIX16_MIN;
    return (fix16_t)result;
}
/* sine of an angle in degrees, Q16.16 in and out */
fix16_t fix16_sin(fix16_t degrees) {
    int negate = 0;

    /* wrap into 0..360 */
    while (degrees < 0)            degrees = fix16_add(degrees, F16(360.0));
    while (degrees >= F16(360.0))  degrees = fix16_sub(degrees, F16(360.0));

    /* fold the bottom half onto the top, remembering the sign */
    if (degrees >= F16(180.0)) {
        degrees = fix16_sub(degrees, F16(180.0));
        negate = 1;
    }

    /* mirror the second quadrant onto the first */
    if (degrees > F16(90.0)) {
        degrees = fix16_sub(F16(180.0), degrees);
    }

    /* which table entry? 256 steps span 90 degrees */
    int64_t position = (((int64_t)degrees * SIN_TABLE_ENTRIES) << 16) / F16(90.0);
    int index = (int)(position >> 16);
        if (index < 0) index = 0;
    if (index >= SIN_TABLE_ENTRIES) {
        /* exactly 90 degrees - return the last entry directly */
        return negate ? fix16_sub(0, sin_table[SIN_TABLE_ENTRIES])
                      : sin_table[SIN_TABLE_ENTRIES];    }

    /* how far between this entry and the next, as a Q16.16 fraction */
    fix16_t fraction = (fix16_t)(position & 0xFFFF);

    fix16_t low  = sin_table[index];
    fix16_t high = sin_table[index + 1];
    fix16_t gap  = fix16_sub(high, low);

    fix16_t result = fix16_add(low, fix16_mul(gap, fraction));
    return negate ? fix16_sub(0, result) : result;
}

/* cos(x) = sin(x + 90) */
fix16_t fix16_cos(fix16_t degrees) {
    return fix16_sin(fix16_add(degrees, F16(90.0)));
}
 /* atan(z) for 0 <= z <= 1, result in degrees.
 * Polynomial from Abramowitz & Stegun 4.4.49, error about 1e-5 rad (0.0006 deg). */
static fix16_t atan_unit_deg(fix16_t z) {
    fix16_t z2 = fix16_mul(z, z);
    fix16_t p  = F16(0.0208351);
    p = fix16_add(F16(-0.0851330), fix16_mul(z2, p));
    p = fix16_add(F16( 0.1801410), fix16_mul(z2, p));
    p = fix16_add(F16(-0.3302995), fix16_mul(z2, p));
    p = fix16_add(F16( 0.9998660), fix16_mul(z2, p));
    fix16_t rad = fix16_mul(z, p);
    return fix16_mul(rad, F16(57.2957795));
}


/* arctangent of y/x, returns degrees in -180..+180 */
fix16_t fix16_atan2(fix16_t y, fix16_t x) {
    if (x == 0 && y == 0) return 0;

    fix16_t ax = (x < 0) ? fix16_sub(0, x) : x;
    fix16_t ay = (y < 0) ? fix16_sub(0, y) : y;    fix16_t angle;

    /* keep the ratio between 0 and 1 */
    if (ay <= ax) angle = atan_unit_deg(fix16_div(ay, ax));
    else          angle = fix16_sub(F16(90.0), atan_unit_deg(fix16_div(ax, ay)));

    if (x < 0) angle = fix16_sub(F16(180.0), angle);   /* left half */
    if (y < 0) angle = fix16_sub(0, angle);            /* lower half */
    return angle;
}

/* square root in Q16.16.
 * sqrt(X/65536) * 65536 = sqrt(X << 16), so shift up and take an integer root.
 * negative input returns 0 rather than failing. */
fix16_t fix16_sqrt(fix16_t x) {
    if (x <= 0) return 0;

    uint64_t num = (uint64_t)x << 16;
    uint64_t res = 0;
    uint64_t bit = (uint64_t)1 << 62;

    while (bit > num) bit >>= 2;           /* start at the highest useful bit */

    while (bit != 0) {
        if (num >= res + bit) {
            num -= res + bit;
            res  = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }

    if (num > res) res++;                  /* round to nearest */
    return (fix16_t)res;
}
