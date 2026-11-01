#ifndef DRIVERS_BAROMETER_MS5611_H
#define DRIVERS_BAROMETER_MS5611_H

#include <drivers/barometer/barometer.h>

#ifdef __cplusplus
extern "C" {
#endif

int ms5611_init(void);
int ms5611_read(baro_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_BAROMETER_MS5611_H */
