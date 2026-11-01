#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_PWM_MOTOR_1 = 0,
    HAL_PWM_MOTOR_2,
    HAL_PWM_MOTOR_3,
    HAL_PWM_MOTOR_4,
    HAL_PWM_COUNT
} hal_pwm_channel_t;

/** Initialise all motor outputs to the safe (disarmed) pulse. */
int hal_pwm_init(void);

/** Pulse width in microseconds, usually 1000..2000. */
int hal_pwm_write_us(hal_pwm_channel_t channel, uint16_t pulse_us);

/** Normalised 0..1 command mapped to the configured arm/disarm range. */
int hal_pwm_write_normalized(hal_pwm_channel_t channel, float value);

/** Force all outputs to the disarmed pulse. Safe to call from a fault path. */
void hal_pwm_disarm_all(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_PWM_H */
