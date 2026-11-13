/*
 * 64-bit division helpers for THEJAS32 (RV32IM).
 *
 * WHY THIS FILE EXISTS
 * The chip divides 32-bit numbers in hardware, but 64-bit division is done by
 * helper functions the compiler calls (__udivdi3 and friends) from libgcc.
 * On this toolchain/board those helpers return WRONG results: on the board,
 * 4310000/1000 gave 18577 and fix16_div(10, 4) gave 0xFFFFFFFF (13 Nov 2026,
 * MATH TEST in app/src/main.c). Root cause not yet found.
 *
 * Defining the helpers here, in our own object file, makes the linker use
 * these instead of libgcc's. They use only 32-bit hardware division and
 * shift-and-subtract, so they don't depend on anything in libgcc.
 *
 * Speed: a full 64-bit divide is a 64-step loop, roughly 10 us at 100 MHz.
 * When both numbers fit in 32 bits it uses the hardware divider instead.
 */
#include <stdint.h>

#ifndef DIV64_PREFIX
#define DIV64_NAME(n) n            /* real build: replace libgcc's helpers */
#else
#define DIV64_NAME(n) test_##n     /* laptop test build: avoid name clash  */
#endif

static uint64_t udivmod64(uint64_t n, uint64_t d, uint64_t *rem)
{
    /* RISC-V convention for divide by zero: quotient all ones, remainder n */
    if (d == 0) {
        if (rem) *rem = n;
        return ~(uint64_t)0;
    }

    /* fast path: both fit in 32 bits -> hardware divide */
    if ((uint32_t)(n >> 32) == 0 && (uint32_t)(d >> 32) == 0) {
        uint32_t n32 = (uint32_t)n, d32 = (uint32_t)d;
        if (rem) *rem = n32 % d32;
        return n32 / d32;
    }

    /* long division, one bit at a time, constant shifts only */
    uint64_t q = 0, r = 0;
    for (int i = 0; i < 64; i++) {
        r = (r << 1) | (n >> 63);
        n <<= 1;
        q <<= 1;
        if (r >= d) {
            r -= d;
            q |= 1;
        }
    }
    if (rem) *rem = r;
    return q;
}

uint64_t DIV64_NAME(__udivdi3)(uint64_t n, uint64_t d)
{
    return udivmod64(n, d, 0);
}

uint64_t DIV64_NAME(__umoddi3)(uint64_t n, uint64_t d)
{
    uint64_t r;
    udivmod64(n, d, &r);
    return r;
}

int64_t DIV64_NAME(__divdi3)(int64_t n, int64_t d)
{
    int neg = (n < 0) ^ (d < 0);
    uint64_t un = (n < 0) ? -(uint64_t)n : (uint64_t)n;
    uint64_t ud = (d < 0) ? -(uint64_t)d : (uint64_t)d;
    if (d == 0) return -1;                    /* RISC-V: x / 0 = -1 */
    uint64_t q = udivmod64(un, ud, 0);
    return neg ? -(int64_t)q : (int64_t)q;
}

int64_t DIV64_NAME(__moddi3)(int64_t n, int64_t d)
{
    uint64_t un = (n < 0) ? -(uint64_t)n : (uint64_t)n;
    uint64_t ud = (d < 0) ? -(uint64_t)d : (uint64_t)d;
    uint64_t r;
    if (d == 0) return n;                     /* RISC-V: x % 0 = x */
    udivmod64(un, ud, &r);
    return (n < 0) ? -(int64_t)r : (int64_t)r;   /* sign follows dividend */
}
