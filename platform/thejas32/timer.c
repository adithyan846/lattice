#include "hal_timer.h"
#include "hal_errno.h"
#include "timer.h"
#include "config.h"
#include "FreeRTOS.h"
#include "task.h"

int hal_timer_init(hal_timer_t timer, const hal_timer_config_t *config) {
    (void)timer; (void)config;
    return 0;
}
int hal_timer_start(hal_timer_t timer) { (void)timer; return 0; }
int hal_timer_stop(hal_timer_t timer) { (void)timer; return 0; }
int hal_timer_deinit(hal_timer_t timer) { (void)timer; return 0; }

uint32_t hal_time_us(void) { return 0; }
uint64_t hal_time_ms(void) { return (uint64_t)xTaskGetTickCount(); }

int hal_timer_delay_us(uint32_t us) {
    timer_put_delay(TIMER_0, (SYSTEM_FREQUENCY / 1000000U) * us);
    return 0;
}
int hal_timer_delay_ms(uint32_t ms) {
    timer_put_delay(TIMER_0, (SYSTEM_FREQUENCY / 1000U) * ms);
    return 0;
}
