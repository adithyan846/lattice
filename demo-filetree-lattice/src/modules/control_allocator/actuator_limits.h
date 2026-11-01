#ifndef MODULES_CONTROL_ALLOCATOR_ACTUATOR_LIMITS_H
#define MODULES_CONTROL_ALLOCATOR_ACTUATOR_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ACTUATOR_LIMITS_MAX_CHANNELS 8

typedef struct {
    float min_value;
    float max_value;
    /** Maximum change per second; <= 0 disables the slew limit. */
    float slew_rate_per_s;
    float previous[ACTUATOR_LIMITS_MAX_CHANNELS];
    int channels;
    int initialized;
} actuator_limits_t;

void actuator_limits_init(actuator_limits_t *limits, int channels, float min_value, float max_value,
                          float slew_rate_per_s);

/**
 * Clamp and slew-limit a motor command array in place. Returns 0 on success,
 * -1 on invalid input.
 */
int actuator_limits_apply(actuator_limits_t *limits, float *commands, int channels, float dt_s);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_CONTROL_ALLOCATOR_ACTUATOR_LIMITS_H */
