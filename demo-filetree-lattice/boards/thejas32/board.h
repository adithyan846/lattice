#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise clocks, pins, buses and safe actuator state. */
int board_init(void);

/** Initialise the board clock tree. Called by board_init(). */
int board_clock_init(void);

/** Reset the SoC. Does not return. */
void board_reset(void);

/** Stable board identifier for logs and GCS. */
uint32_t board_unique_id(void);

const char *board_name(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
