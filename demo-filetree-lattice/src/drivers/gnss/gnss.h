#ifndef DRIVERS_GNSS_GNSS_H
#define DRIVERS_GNSS_GNSS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double lat_deg;
    double lon_deg;
    float alt_m;
    uint8_t fix_type; /**< 0 none, 2 2D, 3 3D */
    uint8_t satellites_used;
    float hdop;
    float vdop;
    float vel_n_mps;
    float vel_e_mps;
    float vel_d_mps;
    float cog_deg;
    bool valid;
    uint64_t timestamp_us;
} gnss_sample_t;

typedef struct {
    const char *name;
    int (*init)(void);
    int (*read)(gnss_sample_t *sample);
} gnss_driver_t;

int gnss_init(void);
int gnss_read(gnss_sample_t *sample);

const gnss_driver_t *gnss_ubx_get(void);
const gnss_driver_t *gnss_nmea_get(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_GNSS_GNSS_H */
