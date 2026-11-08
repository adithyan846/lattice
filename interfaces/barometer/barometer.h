#ifndef LATTICE_BAROMETER_H
#define LATTICE_BAROMETER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Integer units only: soft-float on the THEJAS32 toolchain returns wrong and
 * non-repeatable results, so no float fields are used.
 */
typedef struct
{
    uint32_t pressure_pa;
    int32_t temperature_centi_c;    /* 0.01 C */
    int32_t altitude_cm;            /* above 101325 Pa, filled by the interface */

    uint32_t timestamp_ms;
    bool valid;
} barometer_data_t;

/* Interface lifecycle */
int barometer_init(void);
int barometer_update(void);

/* Sensor data bus operations */
void barometer_publish(const barometer_data_t *data);
bool barometer_get_latest(barometer_data_t *data);

/* Pressure (Pa) to altitude (cm) above standard sea level, integer only. */
int32_t barometer_pressure_to_altitude_cm(uint32_t pressure_pa);

#endif
