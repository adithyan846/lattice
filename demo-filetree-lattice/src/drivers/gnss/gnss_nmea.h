#ifndef DRIVERS_GNSS_GNSS_NMEA_H
#define DRIVERS_GNSS_GNSS_NMEA_H

#include <drivers/gnss/gnss.h>

#ifdef __cplusplus
extern "C" {
#endif

int gnss_nmea_init(void);
int gnss_nmea_read(gnss_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_GNSS_GNSS_NMEA_H */
