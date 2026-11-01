#ifndef DRIVERS_MAGNETOMETER_MAGNETOMETER_H
#define DRIVERS_MAGNETOMETER_MAGNETOMETER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float mag_gauss[3];
    float temperature_c;
    uint64_t timestamp_us;
} mag_sample_t;

typedef struct {
    const char *name;
    int (*init)(void);
    int (*read)(mag_sample_t *sample);
} mag_driver_t;

const mag_driver_t *mag_ist8310_get(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_MAGNETOMETER_MAGNETOMETER_H */
