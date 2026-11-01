#include <drivers/battery/battery_adc.h>

#include <stddef.h>

#include <hal/adc.h>
#include <middleware/time/time.h>

int battery_adc_init(void)
{
    return hal_adc_init();
}

int battery_adc_read(battery_adc_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    sample->voltage_v = hal_adc_read(HAL_ADC_BATTERY_VOLTAGE);
    sample->current_a = hal_adc_read(HAL_ADC_BATTERY_CURRENT);
    sample->timestamp_us = hrt_absolute_time();
    return 0;
}
