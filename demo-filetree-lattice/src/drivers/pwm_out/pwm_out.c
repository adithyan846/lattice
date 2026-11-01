#include <drivers/pwm_out/pwm_out.h>

#include <hal/pwm.h>

int pwm_out_init(void)
{
    const int result = hal_pwm_init();
    hal_pwm_disarm_all();
    return result;
}

int pwm_out_write(const float motors[PWM_OUT_MOTOR_COUNT])
{
    if (motors == NULL) {
        return -1;
    }

    for (int i = 0; i < PWM_OUT_MOTOR_COUNT; i++) {
        float value = motors[i];

        if (value < 0.0f) {
            value = 0.0f;
        } else if (value > 1.0f) {
            value = 1.0f;
        }

        if (hal_pwm_write_normalized((hal_pwm_channel_t)i, value) != 0) {
            return -1;
        }
    }

    return 0;
}

void pwm_out_disarm(void)
{
    hal_pwm_disarm_all();
}
