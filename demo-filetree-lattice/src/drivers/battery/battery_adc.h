#ifndef DRIVERS_BATTERY_BATTERY_ADC_H
#define DRIVERS_BATTERY_BATTERY_ADC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float voltage_v;
    float current_a;
    uint64_t timestamp_us;
} battery_adc_sample_t;

int battery_adc_init(void);
int battery_adc_read(battery_adc_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_BATTERY_BATTERY_ADC_H */
