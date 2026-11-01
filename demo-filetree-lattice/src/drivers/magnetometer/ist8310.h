#ifndef DRIVERS_MAGNETOMETER_IST8310_H
#define DRIVERS_MAGNETOMETER_IST8310_H

#include <drivers/magnetometer/magnetometer.h>

#ifdef __cplusplus
extern "C" {
#endif

int ist8310_init(void);
int ist8310_read(mag_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_MAGNETOMETER_IST8310_H */
