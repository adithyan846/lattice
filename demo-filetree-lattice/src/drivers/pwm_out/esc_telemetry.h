#ifndef DRIVERS_PWM_OUT_ESC_TELEMETRY_H
#define DRIVERS_PWM_OUT_ESC_TELEMETRY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float voltage_v;
    float current_a;
    float temperature_c;
    uint32_t erpm;
    uint8_t esc_index;
} esc_telemetry_t;

int esc_telemetry_init(void);
int esc_telemetry_read(esc_telemetry_t *telemetry);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_PWM_OUT_ESC_TELEMETRY_H */
