#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_PWM_CH_0 = 0, HAL_PWM_CH_1, HAL_PWM_CH_2, HAL_PWM_CH_3,
    HAL_PWM_CH_4, HAL_PWM_CH_5, HAL_PWM_CH_6, HAL_PWM_CH_7,
} hal_pwm_ch_t;

typedef enum {
    HAL_PWM_MODE_CONTINUOUS = 0,
    HAL_PWM_MODE_ONE_SHOT   = 1,
} hal_pwm_mode_t;

typedef enum {
    HAL_PWM_ALIGN_LEFT  = 0,
    HAL_PWM_ALIGN_RIGHT = 1,
} hal_pwm_align_t;

typedef struct {
    hal_pwm_mode_t mode;
    hal_pwm_align_t align;
    uint32_t freq_hz;
    uint32_t duty_cycle_pct;
    bool output_polarity_high;
} hal_pwm_config_t;

int hal_pwm_init(hal_pwm_ch_t ch, const hal_pwm_config_t *config);
int hal_pwm_set_duty(hal_pwm_ch_t ch, uint32_t duty_cycle_pct);
int hal_pwm_set_freq(hal_pwm_ch_t ch, uint32_t freq_hz);
int hal_pwm_start(hal_pwm_ch_t ch);
int hal_pwm_stop(hal_pwm_ch_t ch);
int hal_pwm_deinit(hal_pwm_ch_t ch);
