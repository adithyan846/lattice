#include "board.h"
#include "hal_gpio.h"
#include "hal_spi.h"

/*
 * Both sensors run SPI mode 3, MSB first, 8-bit, hardware CS0 held with
 * CSAAT, at 100 MHz / 32 = 3.125 MHz (BMP280 and LSM6DSO limit is 10 MHz).
 */
static const hal_spi_config_t board_sensor_spi_cfg = {
    .mode = HAL_SPI_MODE_3,
    .bit_order = HAL_SPI_BIT_ORDER_MSB,
    .cs_mode = HAL_SPI_CS_MODE_CSAAT,
    .hw_cs = HAL_SPI_CS_HW_0,
    .freq_hz = 0,
    .baud_div = HAL_SPI_BAUD_DIV_32,
    .word_size = 8,
};

int board_init(void) {
    int ret;

    ret = hal_gpio_init();
    if (ret) return ret;

    ret = hal_spi_init(BOARD_SPI_IMU_BUS, &board_sensor_spi_cfg);
    if (ret) return ret;

    return hal_spi_init(BOARD_SPI_BARO_BUS, &board_sensor_spi_cfg);
}
