#ifndef LATTICE_BAROMETER_H
#define LATTICE_BAROMETER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    float pressure_pa;
    float temperature_c;
    uint32_t timestamp_ms;
    bool valid;
} barometer_data_t;

/* Interface lifecycle */
int barometer_init(void);
int barometer_update(void);

/* Sensor data bus operations */
void barometer_publish(const barometer_data_t *data);
bool barometer_get_latest(barometer_data_t *data);

#endif
