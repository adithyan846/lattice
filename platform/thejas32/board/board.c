#include "board.h"
#include "hal_gpio.h"

void board_init(void) {
    hal_gpio_init();
}
