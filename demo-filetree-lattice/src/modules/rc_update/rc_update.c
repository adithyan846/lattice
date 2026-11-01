#include <modules/rc_update/rc_update.h>

#include <stddef.h>

#include <middleware/log/log.h>

static float normalize_stick(uint16_t pwm)
{
    float value = ((float)pwm - 1500.0f) / 500.0f;

    if (value > 1.0f) {
        value = 1.0f;
    } else if (value < -1.0f) {
        value = -1.0f;
    }

    return value;
}

static uint8_t mode_from_switch(uint16_t pwm)
{
    if (pwm < 1300) {
        return 0; /* stabilized */

    } else if (pwm < 1700) {
        return 1; /* altitude hold */

    } else {
        return 2; /* position hold */
    }
}

int rc_update_map(const rc_channels_s *channels, rc_input_t *input)
{
    if (channels == NULL || input == NULL || !channels->valid || channels->channel_count < 6) {
        return -1;
    }

    input->roll = normalize_stick(channels->channels[0]);
    input->pitch = normalize_stick(channels->channels[1]);
    input->thrust = normalize_stick(channels->channels[2]);
    input->yaw = normalize_stick(channels->channels[3]);
    input->arm_switch = channels->channels[4] > 1500;
    input->mode_switch = mode_from_switch(channels->channels[5]);
    input->valid = true;
    return 0;
}

int rc_update_start(void)
{
    /* TODO: subscribe to rc_channels, apply calibration/deadband/expo, and
     * publish manual_control_setpoint plus failsafe_flags.rc_loss. */
    FRT_LOG_WARN("rc_update: scaffold only, RC task not implemented");
    return -1;
}

int rc_update_stop(void)
{
    return 0;
}
