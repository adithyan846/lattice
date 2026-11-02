#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_TIMER_0 = 0,
    HAL_TIMER_1 = 1,
    HAL_TIMER_2 = 2,
} hal_timer_t;

typedef enum {
    HAL_TIMER_MODE_ONESHOT  = 0,
    HAL_TIMER_MODE_PERIODIC = 1,
} hal_timer_mode_t;

typedef void (*hal_timer_cb_t)(void *arg);

typedef struct {
    hal_timer_mode_t mode;
    uint32_t period_us;
    hal_timer_cb_t callback;
    void *arg;
} hal_timer_config_t;

int hal_timer_init(hal_timer_t timer, const hal_timer_config_t *config);
int hal_timer_start(hal_timer_t timer);
int hal_timer_stop(hal_timer_t timer);
int hal_timer_deinit(hal_timer_t timer);

uint32_t hal_time_us(void);
uint64_t hal_time_ms(void);

int hal_timer_delay_us(uint32_t us);
int hal_timer_delay_ms(uint32_t ms);
