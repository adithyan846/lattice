#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_ADC_CH_0 = 0,
    HAL_ADC_CH_1 = 1,
    HAL_ADC_CH_2 = 2,
    HAL_ADC_CH_3 = 3,
} hal_adc_ch_t;

int hal_adc_init(void);
int hal_adc_read(hal_adc_ch_t ch, uint32_t *out_value);
int hal_adc_deinit(void);
