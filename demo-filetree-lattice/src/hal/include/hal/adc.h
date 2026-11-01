#ifndef HAL_ADC_H
#define HAL_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_ADC_BATTERY_VOLTAGE = 0,
    HAL_ADC_BATTERY_CURRENT,
    HAL_ADC_BOARD_TEMPERATURE,
    HAL_ADC_COUNT
} hal_adc_channel_t;

int hal_adc_init(void);

/** Return a calibrated value in volts (or the channel's native unit). */
float hal_adc_read(hal_adc_channel_t channel);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ADC_H */
