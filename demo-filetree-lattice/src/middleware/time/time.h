#ifndef MIDDLEWARE_TIME_TIME_H
#define MIDDLEWARE_TIME_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Microseconds since boot (monotonic). */
uint64_t hrt_absolute_time(void);

/** Microseconds at boot, for log alignment. */
uint64_t hrt_boot_time(void);

void hrt_init(void);

/** Elapsed microseconds since `since_us`, safe across wrap on 32-bit ticks. */
uint64_t hrt_elapsed_time(uint64_t since_us);

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_TIME_TIME_H */
