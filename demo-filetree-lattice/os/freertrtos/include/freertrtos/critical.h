#ifndef FREERTRTOS_CRITICAL_H
#define FREERTRTOS_CRITICAL_H

/**
 * Short critical sections. Never call a driver, take a mutex or do float math
 * inside one. FreeRTOS critical sections nest.
 */

#ifdef __cplusplus
extern "C" {
#endif

void frt_critical_enter(void);
void frt_critical_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* FREERTRTOS_CRITICAL_H */
