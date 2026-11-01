#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int board_init(void);
int board_clock_init(void);
void board_reset(void);
uint32_t board_unique_id(void);
const char *board_name(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
