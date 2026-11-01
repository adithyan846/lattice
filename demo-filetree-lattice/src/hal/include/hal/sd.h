#ifndef HAL_SD_H
#define HAL_SD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_SD_BLOCK_SIZE 512u

int hal_sd_init(void);
bool hal_sd_present(void);
int hal_sd_read_blocks(uint32_t block, void *buffer, uint32_t block_count);
int hal_sd_write_blocks(uint32_t block, const void *buffer, uint32_t block_count);
uint64_t hal_sd_capacity_blocks(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SD_H */
