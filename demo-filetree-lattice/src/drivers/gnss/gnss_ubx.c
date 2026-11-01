#include <drivers/gnss/gnss_ubx.h>

#include <stddef.h>

#include <hal/uart.h>

/* UBX sync characters and message classes used by the parser. */
#define UBX_SYNC_1 0xB5u
#define UBX_SYNC_2 0x62u
#define UBX_CLASS_NAV 0x01u
#define UBX_ID_PVT 0x07u

int gnss_ubx_init(void)
{
    /* TODO: configure the receiver for UBX-only output and the desired rate
     * (e.g. NAV-PVT at 10 Hz) over HAL_UART_GNSS. */
    return -1;
}

int gnss_ubx_read(gnss_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    /* TODO: byte-stream parser with sync, length, payload and checksum;
     * decode NAV-PVT into lat/lon/alt/fix/satellites/velocity. */
    return -1;
}

static const gnss_driver_t gnss_ubx_driver = {
    .name = "ubx",
    .init = gnss_ubx_init,
    .read = gnss_ubx_read,
};

const gnss_driver_t *gnss_ubx_get(void)
{
    return &gnss_ubx_driver;
}
