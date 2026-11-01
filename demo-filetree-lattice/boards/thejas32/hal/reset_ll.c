#include <hal/reset.h>

/* TODO: write the SoC reset register. */
void hal_system_reset(void)
{
    for (;;) {
        /* Wait for the watchdog or a debugger. */
    }
}

uint32_t hal_reset_cause(void)
{
    return 0;
}
