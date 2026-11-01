#include <hal/sd.h>

/* TODO: implement SD block access (SPI or SDIO) for flight logging. */

int hal_sd_init(void)
{
    return -1;
}

bool hal_sd_present(void)
{
    return false;
}

int hal_sd_read_blocks(uint32_t block, void *buffer, uint32_t block_count)
{
    (void)block;
    (void)buffer;
    (void)block_count;
    return -1;
}

int hal_sd_write_blocks(uint32_t block, const void *buffer, uint32_t block_count)
{
    (void)block;
    (void)buffer;
    (void)block_count;
    return -1;
}

uint64_t hal_sd_capacity_blocks(void)
{
    return 0;
}
