#ifndef FREERTRTOS_TIME_H
#define FREERTRTOS_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Monotonic time since boot, microseconds. */
uint64_t frt_time_us(void);

/** Monotonic time since boot, milliseconds. */
uint64_t frt_time_ms(void);

/** Busy-wait or kernel delay for a short interval. */
void frt_delay_us(uint32_t delay_us);

/** Convert milliseconds to kernel ticks. */
uint32_t frt_ms_to_ticks(uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* FREERTRTOS_TIME_H */
