#ifndef MODULES_RC_UPDATE_RC_UPDATE_H
#define MODULES_RC_UPDATE_RC_UPDATE_H

#include <stdbool.h>
#include <stdint.h>

#include <msg/generated/uorb_topics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float roll;
    float pitch;
    float yaw;
    float thrust;
    uint8_t mode_switch;
    bool arm_switch;
    bool valid;
} rc_input_t;

/**
 * Normalise raw RC channels (1000-2000 us) into [-1, 1] sticks. Channel order
 * is roll, pitch, thrust, yaw, arm switch, mode switch. Returns 0 on success.
 */
int rc_update_map(const rc_channels_s *channels, rc_input_t *input);

int rc_update_start(void);
int rc_update_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_RC_UPDATE_RC_UPDATE_H */
