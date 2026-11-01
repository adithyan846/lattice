#ifndef DRIVERS_BAROMETER_BAROMETER_H
#define DRIVERS_BAROMETER_BAROMETER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float pressure_pa;
    float temperature_c;
    uint64_t timestamp_us;
} baro_sample_t;

typedef struct {
    const char *name;
    int (*init)(void);
    int (*read)(baro_sample_t *sample);
} baro_driver_t;

const baro_driver_t *baro_ms5611_get(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_BAROMETER_BAROMETER_H */
