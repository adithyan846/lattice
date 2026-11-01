#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_FLASH_PARTITION_PARAMS = 0,
    HAL_FLASH_PARTITION_MISSION,
    HAL_FLASH_PARTITION_BOOTLOG,
    HAL_FLASH_PARTITION_COUNT
} hal_flash_partition_t;

int hal_flash_init(void);
int hal_flash_read(hal_flash_partition_t partition, uint32_t offset, void *data,
                   size_t length);
int hal_flash_write(hal_flash_partition_t partition, uint32_t offset, const void *data,
                    size_t length);
int hal_flash_erase(hal_flash_partition_t partition);
size_t hal_flash_partition_size(hal_flash_partition_t partition);

#ifdef __cplusplus
}
#endif

#endif /* HAL_FLASH_H */
