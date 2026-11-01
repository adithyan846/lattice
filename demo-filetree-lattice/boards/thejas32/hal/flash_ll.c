#include <hal/flash.h>

/* TODO: implement parameter/mission partitions on the THEJAS32 flash. */

int hal_flash_init(void)
{
    return -1;
}

int hal_flash_read(hal_flash_partition_t partition, uint32_t offset, void *data,
                   size_t length)
{
    (void)partition;
    (void)offset;
    (void)data;
    (void)length;
    return -1;
}

int hal_flash_write(hal_flash_partition_t partition, uint32_t offset, const void *data,
                    size_t length)
{
    (void)partition;
    (void)offset;
    (void)data;
    (void)length;
    return -1;
}

int hal_flash_erase(hal_flash_partition_t partition)
{
    (void)partition;
    return -1;
}

size_t hal_flash_partition_size(hal_flash_partition_t partition)
{
    (void)partition;
    return 0;
}
