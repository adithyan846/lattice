#include <drivers/pwm_out/esc_telemetry.h>

#include <stddef.h>

/* TODO: decode DShot telemetry or a separate ESC telemetry UART. */

int esc_telemetry_init(void)
{
    return -1;
}

int esc_telemetry_read(esc_telemetry_t *telemetry)
{
    (void)telemetry;
    return -1;
}
