#include <middleware/time/time.h>

#include <freertrtos/time.h>

void hrt_init(void)
{
    /* The freertrtos tick is the time base; nothing to configure yet. If a
     * high-resolution timer is added, latch its epoch here. */
}

uint64_t hrt_absolute_time(void)
{
    return frt_time_us();
}

uint64_t hrt_boot_time(void)
{
    return 0;
}

uint64_t hrt_elapsed_time(uint64_t since_us)
{
    const uint64_t now = hrt_absolute_time();
    return now >= since_us ? now - since_us : 0;
}
