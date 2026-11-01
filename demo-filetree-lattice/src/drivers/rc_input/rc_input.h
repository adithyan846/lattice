#ifndef DRIVERS_RC_INPUT_RC_INPUT_H
#define DRIVERS_RC_INPUT_RC_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RC_MAX_CHANNELS 16

typedef struct {
    uint16_t channels[RC_MAX_CHANNELS];
    uint8_t channel_count;
    bool valid;
    uint64_t timestamp_us;
} rc_sample_t;

typedef struct {
    const char *name;
    int (*init)(void);
    int (*read)(rc_sample_t *sample);
} rc_driver_t;

const rc_driver_t *rc_sbus_get(void);
const rc_driver_t *rc_crsf_get(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_RC_INPUT_RC_INPUT_H */
