#include "hal_timer.h"
#include "hal_errno.h"
#include "timer.h"
#include "config.h"
#include <stdbool.h>

/*
 * Timebase for hal_time_us() / hal_time_ms()
 * -------------------------------------------
 * TIMER_2 runs free at the system clock (100 MHz, 10 ns per count),
 * counting DOWN from 0xFFFFFFFF and wrapping every ~42.9 s.
 * Each call works out how many counts passed since the previous call and
 * adds them to a 64-bit microsecond total, so time keeps going past the wrap.
 *
 * TIMER_0 stays free for hal_timer_delay_*(), which reprograms it each call.
 *
 * Rule: something must call hal_time_us() or hal_time_ms() at least once
 * every 42 s, or one wrap is missed. The flight loop calls it 500x a second.
 *
 * hal_time_us() returns 32 bits, which wraps after ~71 minutes. Always take
 * differences as (uint32_t)(now - then): unsigned subtraction stays correct
 * across that wrap.
 */

#define TIMEBASE_TIMER      TIMER_2
#define COUNTS_PER_US       (SYSTEM_FREQUENCY / 1000000U)     /* 100 */

static bool     tb_started   = false;
static uint32_t tb_last      = 0;       /* timer value at the previous read */
static uint32_t tb_leftover  = 0;       /* counts not yet worth a whole us  */
static uint64_t tb_total_us  = 0;

/* Disable interrupts, remembering whether they were on. Works with or
 * without FreeRTOS running. mstatus bit 3 (MIE) is the global enable.   */
static inline uint32_t irq_save(void) {
    uint32_t m;
    __asm__ volatile ("csrrci %0, mstatus, 8" : "=r"(m) :: "memory");
    return m;
}
static inline void irq_restore(uint32_t m) {
    if (m & 8) __asm__ volatile ("csrsi mstatus, 8" ::: "memory");
}

static void timebase_start(void) {
    Timer(TIMEBASE_TIMER).Control   = 0x0;          /* stop                     */
    Timer(TIMEBASE_TIMER).LoadCount = 0xFFFFFFFFu;  /* count down from the top  */
    Timer(TIMEBASE_TIMER).Control   = 0x07;         /* enable, interrupt masked */
    tb_last     = Timer(TIMEBASE_TIMER).CurrentValue;
    tb_leftover = 0;
    tb_total_us = 0;
    tb_started  = true;
}

static uint64_t timebase_now_us(void) {
    uint32_t irq = irq_save();

    if (!tb_started) timebase_start();

    uint32_t now   = Timer(TIMEBASE_TIMER).CurrentValue;
    uint32_t delta = tb_last - now;                 /* counting down; wraps cleanly */
    tb_last = now;

    uint32_t whole = delta / COUNTS_PER_US;
    uint32_t rem   = delta % COUNTS_PER_US + tb_leftover;
    if (rem >= COUNTS_PER_US) { whole++; rem -= COUNTS_PER_US; }
    tb_leftover  = rem;
    tb_total_us += whole;

    uint64_t t = tb_total_us;
    irq_restore(irq);
    return t;
}

int hal_timer_init(hal_timer_t timer, const hal_timer_config_t *config) {
    (void)timer; (void)config;
    return 0;
}
int hal_timer_start(hal_timer_t timer) { (void)timer; return 0; }
int hal_timer_stop(hal_timer_t timer) { (void)timer; return 0; }
int hal_timer_deinit(hal_timer_t timer) { (void)timer; return 0; }

uint32_t hal_time_us(void) { return (uint32_t)timebase_now_us(); }
uint64_t hal_time_ms(void) { return timebase_now_us() / 1000U; }

int hal_timer_delay_us(uint32_t us) {
    timer_put_delay(TIMER_0, COUNTS_PER_US * us);
    return 0;
}
int hal_timer_delay_ms(uint32_t ms) {
    timer_put_delay(TIMER_0, (SYSTEM_FREQUENCY / 1000U) * ms);
    return 0;
}
