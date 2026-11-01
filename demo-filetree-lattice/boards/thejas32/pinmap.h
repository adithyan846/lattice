#ifndef BOARD_PINMAP_H
#define BOARD_PINMAP_H

/*
 * Reference pin map for the THEJAS32 board.
 *
 * These values are placeholders. Replace them with the pin numbers from your
 * schematic and record the alternate-function numbers next to each entry.
 * A wrong pin map is one of the most common bring-up failures.
 */

/* Console UART */
#define BOARD_UART_CONSOLE_TX_PIN 0u
#define BOARD_UART_CONSOLE_RX_PIN 1u

/* GNSS UART */
#define BOARD_UART_GNSS_TX_PIN    2u
#define BOARD_UART_GNSS_RX_PIN    3u

/* Telemetry UART */
#define BOARD_UART_TELEM_TX_PIN   4u
#define BOARD_UART_TELEM_RX_PIN   5u

/* IMU SPI */
#define BOARD_SPI_IMU_SCK_PIN     6u
#define BOARD_SPI_IMU_MOSI_PIN    7u
#define BOARD_SPI_IMU_MISO_PIN    8u
#define BOARD_SPI_IMU_CS_PIN      9u
#define BOARD_IMU_DRDY_PIN        10u

/* Magnetometer I2C */
#define BOARD_I2C_MAG_SCL_PIN     11u
#define BOARD_I2C_MAG_SDA_PIN     12u

/* Motor outputs: timer channel mapping is board-specific. */
#define BOARD_PWM_MOTOR_1_PIN     13u
#define BOARD_PWM_MOTOR_2_PIN     14u
#define BOARD_PWM_MOTOR_3_PIN     15u
#define BOARD_PWM_MOTOR_4_PIN     16u

/* Battery ADC */
#define BOARD_ADC_VBAT_PIN        17u
#define BOARD_ADC_CURRENT_PIN     18u

/* Switches and LEDs */
#define BOARD_SAFETY_SWITCH_PIN   19u
#define BOARD_KILL_SWITCH_PIN     20u
#define BOARD_LED_STATUS_PIN      21u

#endif /* BOARD_PINMAP_H */
