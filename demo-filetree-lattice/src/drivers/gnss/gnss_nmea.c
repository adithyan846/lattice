#include <drivers/gnss/gnss_nmea.h>

#include <stddef.h>

#include <hal/uart.h>

int gnss_nmea_init(void)
{
    /* TODO: configure the receiver for NMEA output if UBX is not used. */
    return -1;
}

int gnss_nmea_read(gnss_sample_t *sample)
{
    if (sample == NULL) {
        return -1;
    }

    /* TODO: parse GGA (fix/alt/satellites/HDOP) and RMC/VTG (velocity/course)
     * sentences with checksum validation. */
    return -1;
}

static const gnss_driver_t gnss_nmea_driver = {
    .name = "nmea",
    .init = gnss_nmea_init,
    .read = gnss_nmea_read,
};

const gnss_driver_t *gnss_nmea_get(void)
{
    return &gnss_nmea_driver;
}
