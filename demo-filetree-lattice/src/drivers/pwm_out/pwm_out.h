#ifndef DRIVERS_PWM_OUT_PWM_OUT_H
#define DRIVERS_PWM_OUT_PWM_OUT_H

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_OUT_MOTOR_COUNT 4

/** Initialise the ESC outputs in the disarmed state. */
int pwm_out_init(void);

/** Write normalised motor commands (0..1). Values are clamped. */
int pwm_out_write(const float motors[PWM_OUT_MOTOR_COUNT]);

/** Force all outputs to the disarmed pulse. */
void pwm_out_disarm(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_PWM_OUT_PWM_OUT_H */
