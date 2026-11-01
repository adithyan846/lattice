#ifndef DRIVERS_STORAGE_STORAGE_SD_H
#define DRIVERS_STORAGE_STORAGE_SD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int storage_sd_init(void);
bool storage_sd_present(void);
int storage_sd_read_blocks(uint32_t block, void *buffer, uint32_t block_count);
int storage_sd_write_blocks(uint32_t block, const void *buffer, uint32_t block_count);
uint64_t storage_sd_capacity_blocks(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_STORAGE_STORAGE_SD_H */
