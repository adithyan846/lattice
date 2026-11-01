#ifndef MODULES_BATTERY_STATUS_BATTERY_STATUS_H
#define MODULES_BATTERY_STATUS_BATTERY_STATUS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float voltage_v;
    float current_a;
    float discharged_mah;
    float remaining;
    float cell_voltage_low;
    float cell_voltage_critical;
    float capacity_mah;
    uint8_t cell_count;
    bool low_voltage;
    bool critical_voltage;
} battery_monitor_t;

void battery_monitor_init(battery_monitor_t *monitor, uint8_t cell_count, float capacity_mah);

/**
 * Integrate current and estimate remaining charge from the per-cell voltage.
 * `dt_s` is in seconds. Returns the updated remaining fraction [0, 1].
 */
float battery_monitor_update(battery_monitor_t *monitor, float voltage_v, float current_a,
                             float dt_s);

int battery_status_start(void);
int battery_status_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_BATTERY_STATUS_BATTERY_STATUS_H */
