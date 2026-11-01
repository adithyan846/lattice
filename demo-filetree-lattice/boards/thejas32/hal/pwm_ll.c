#include <hal/pwm.h>

/* TODO: implement against the THEJAS32 timer/PWM peripheral.
 *
 * Safety: the init function must leave all outputs at the disarmed pulse
 * before any driver can write a throttle value. */

int hal_pwm_init(void)
{
    return -1;
}

int hal_pwm_write_us(hal_pwm_channel_t channel, uint16_t pulse_us)
{
    (void)channel;
    (void)pulse_us;
    return -1;
}

int hal_pwm_write_normalized(hal_pwm_channel_t channel, float value)
{
    (void)channel;
    (void)value;
    return -1;
}

void hal_pwm_disarm_all(void)
{
    /* TODO: write the disarmed pulse to every channel without blocking. */
}
