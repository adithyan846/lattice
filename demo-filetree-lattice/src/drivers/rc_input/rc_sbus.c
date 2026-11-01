#include <drivers/rc_input/rc_sbus.h>

#include <stddef.h>

#include <hal/uart.h>

#define SBUS_FRAME_LENGTH 25u
#define SBUS_START_BYTE 0x0Fu
#define SBUS_END_BYTE 0x00u
#define SBUS_FAILSAFE_FLAG 0x08u
#define SBUS_FRAMELOST_FLAG 0x04u

int rc_sbus_init(void)
{
    /* TODO: configure HAL_UART_RC for 100000 baud, 8E2, inverted if required. */
    return -1;
}

int rc_sbus_read(rc_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    /* TODO: read 25-byte frames, validate start/end bytes, unpack 11-bit
     * channels, map flags to `valid`, fill timestamp. */
    return -1;
}

static const rc_driver_t rc_sbus_driver = {
    .name = "sbus",
    .init = rc_sbus_init,
    .read = rc_sbus_read,
};

const rc_driver_t *rc_sbus_get(void)
{
    return &rc_sbus_driver;
}
