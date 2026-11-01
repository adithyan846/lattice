#include <drivers/gnss/gnss.h>

#include <stddef.h>

/* Select the receiver protocol at build time or by probing. UBX is the default
 * because it gives a compact, well-defined NAV-PVT message. */
static const gnss_driver_t *active_driver;

int gnss_init(void)
{
    active_driver = gnss_ubx_get();
    return active_driver != NULL ? active_driver->init() : -1;
}

int gnss_read(gnss_sample_t *sample)
{
    if (active_driver == NULL) {
        return -1;
    }

    return active_driver->read(sample);
}
