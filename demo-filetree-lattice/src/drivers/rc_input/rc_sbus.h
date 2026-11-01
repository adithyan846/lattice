#ifndef DRIVERS_RC_INPUT_RC_SBUS_H
#define DRIVERS_RC_INPUT_RC_SBUS_H

#include <drivers/rc_input/rc_input.h>

#ifdef __cplusplus
extern "C" {
#endif

int rc_sbus_init(void);
int rc_sbus_read(rc_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_RC_INPUT_RC_SBUS_H */
