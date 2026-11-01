#ifndef HAL_HAL_H
#define HAL_HAL_H

/**
 * Hardware abstraction interfaces.
 *
 * Board support packages implement these functions. Drivers and modules only
 * include this directory; they never touch a register or a board header.
 */

#include <hal/adc.h>
#include <hal/flash.h>
#include <hal/gpio.h>
#include <hal/i2c.h>
#include <hal/pwm.h>
#include <hal/reset.h>
#include <hal/sd.h>
#include <hal/spi.h>
#include <hal/uart.h>

#endif /* HAL_HAL_H */
