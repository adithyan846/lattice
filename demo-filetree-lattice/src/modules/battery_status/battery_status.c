#include <modules/battery_status/battery_status.h>

#include <stddef.h>

#include <middleware/log/log.h>

#define LIPO_CELL_EMPTY_V 3.30f
#define LIPO_CELL_FULL_V 4.20f
#define LIPO_CELL_LOW_V 3.50f
#define LIPO_CELL_CRITICAL_V 3.30f

static float clampf(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

void battery_monitor_init(battery_monitor_t *monitor, uint8_t cell_count, float capacity_mah)
{
    if (monitor == NULL) {
        return;
    }

    *monitor = (battery_monitor_t){0};
    monitor->cell_count = cell_count > 0 ? cell_count : 4;
    monitor->capacity_mah = capacity_mah > 0.0f ? capacity_mah : 5000.0f;
    monitor->cell_voltage_low = LIPO_CELL_LOW_V;
    monitor->cell_voltage_critical = LIPO_CELL_CRITICAL_V;
}

float battery_monitor_update(battery_monitor_t *monitor, float voltage_v, float current_a,
                             float dt_s)
{
    if (monitor == NULL) {
        return 0.0f;
    }

    const float dt = dt_s > 0.0f ? dt_s : 0.0f;

    monitor->voltage_v = voltage_v;
    monitor->current_a = current_a;

    if (monitor->current_a > 0.0f) {
        monitor->discharged_mah += monitor->current_a * dt / 3.6f;
    }

    /* A simple linear LiPo approximation; replace with an OCV curve after
     * characterising the pack. */
    const float cell_voltage = voltage_v / (float)monitor->cell_count;
    const float voltage_remaining =
        (cell_voltage - LIPO_CELL_EMPTY_V) / (LIPO_CELL_FULL_V - LIPO_CELL_EMPTY_V);
    const float coulomb_remaining =
        monitor->capacity_mah > 0.0f ? 1.0f - monitor->discharged_mah / monitor->capacity_mah : 0.0f;

    monitor->remaining = clampf(voltage_remaining < coulomb_remaining ? voltage_remaining
                                                                      : coulomb_remaining,
                                0.0f, 1.0f);
    monitor->low_voltage = cell_voltage <= monitor->cell_voltage_low;
    monitor->critical_voltage = cell_voltage <= monitor->cell_voltage_critical;
    return monitor->remaining;
}

int battery_status_start(void)
{
    /* TODO: subscribe to the ADC battery driver, run the monitor at 10 Hz, and
     * publish battery_status. */
    FRT_LOG_WARN("battery_status: scaffold only, ADC task not implemented");
    return -1;
}

int battery_status_stop(void)
{
    return 0;
}
