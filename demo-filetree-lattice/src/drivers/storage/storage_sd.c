#include <drivers/storage/storage_sd.h>

#include <hal/sd.h>

int storage_sd_init(void)
{
    return hal_sd_init();
}

bool storage_sd_present(void)
{
    return hal_sd_present();
}

int storage_sd_read_blocks(uint32_t block, void *buffer, uint32_t block_count)
{
    return hal_sd_read_blocks(block, buffer, block_count);
}

int storage_sd_write_blocks(uint32_t block, const void *buffer, uint32_t block_count)
{
    return hal_sd_write_blocks(block, buffer, block_count);
}

uint64_t storage_sd_capacity_blocks(void)
{
    return hal_sd_capacity_blocks();
}
