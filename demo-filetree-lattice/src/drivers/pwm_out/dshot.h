#ifndef DRIVERS_PWM_OUT_DSHOT_H
#define DRIVERS_PWM_OUT_DSHOT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSHOT_MIN_THROTTLE 0u
#define DSHOT_MAX_THROTTLE 2047u
#define DSHOT_DISARMED 0u

/**
 * Encode a DShot frame (11-bit throttle + optional telemetry request + 4-bit
 * CRC). The caller is responsible for bit-banging or feeding the DMA engine.
 */
uint16_t dshot_encode(float throttle_normalized, bool request_telemetry);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_PWM_OUT_DSHOT_H */
