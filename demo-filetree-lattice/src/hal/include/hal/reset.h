#ifndef HAL_RESET_H
#define HAL_RESET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Reset the SoC. Does not return. */
void hal_system_reset(void);

/** Reset cause for diagnostics: 0 = power-on, 1 = watchdog, 2 = software. */
uint32_t hal_reset_cause(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_RESET_H */
