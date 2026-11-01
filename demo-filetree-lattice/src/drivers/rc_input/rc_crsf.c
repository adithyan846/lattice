#include <drivers/rc_input/rc_crsf.h>

#include <stddef.h>

#include <hal/uart.h>

#define CRSF_SYNC_BYTE 0xC8u
#define CRSF_MAX_FRAME 64u

int rc_crsf_init(void)
{
    /* TODO: configure HAL_UART_RC for 420000 baud, 8N1. */
    return -1;
}

int rc_crsf_read(rc_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    /* TODO: parse CRSF frames (address, length, type, payload, CRC), decode
     * RC channels and link statistics, fill timestamp. */
    return -1;
}

static const rc_driver_t rc_crsf_driver = {
    .name = "crsf",
    .init = rc_crsf_init,
    .read = rc_crsf_read,
};

const rc_driver_t *rc_crsf_get(void)
{
    return &rc_crsf_driver;
}
