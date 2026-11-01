#include <modules/control_allocator/actuator_limits.h>

#include <stddef.h>

static float clampf(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

void actuator_limits_init(actuator_limits_t *limits, int channels, float min_value, float max_value,
                          float slew_rate_per_s)
{
    if (limits == NULL) {
        return;
    }

    if (channels < 0) {
        channels = 0;

    } else if (channels > ACTUATOR_LIMITS_MAX_CHANNELS) {
        channels = ACTUATOR_LIMITS_MAX_CHANNELS;
    }

    limits->min_value = min_value;
    limits->max_value = max_value;
    limits->slew_rate_per_s = slew_rate_per_s;
    limits->channels = channels;
    limits->initialized = 1;

    for (int channel = 0; channel < ACTUATOR_LIMITS_MAX_CHANNELS; channel++) {
        limits->previous[channel] = min_value;
    }
}

int actuator_limits_apply(actuator_limits_t *limits, float *commands, int channels, float dt_s)
{
    if (limits == NULL || commands == NULL || !limits->initialized || channels <= 0 ||
        channels > limits->channels || dt_s <= 0.0f) {
        return -1;
    }

    for (int channel = 0; channel < channels; channel++) {
        float command = clampf(commands[channel], limits->min_value, limits->max_value);

        if (limits->slew_rate_per_s > 0.0f) {
            const float max_delta = limits->slew_rate_per_s * dt_s;
            const float delta = command - limits->previous[channel];

            if (delta > max_delta) {
                command = limits->previous[channel] + max_delta;

            } else if (delta < -max_delta) {
                command = limits->previous[channel] - max_delta;
            }
        }

        commands[channel] = command;
        limits->previous[channel] = command;
    }

    return 0;
}
