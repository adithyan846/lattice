# HAL vs BSP — Comprehensive Analysis & Refinement Plan

**Date:** 2026-11-03  
**Scope:** All HAL modules (SPI, GPIO, UART, Timer) vs VEGA BSP

---

## 1. SPI HAL — Remaining Issues

### 1.1 Baud Rate Inaccuracy
**Problem:** Hz → divider mapping picks closest divider, but may be significantly off.

| Requested | Divider Picked | Actual Freq | Error |
|-----------|---------------|-------------|-------|
| 1 MHz | 128 (÷128) | 781.25 kHz | -22% |
| 5 MHz | 16 (÷16) | 6.25 MHz | +25% |
| 10 MHz | 8 (÷8) | 12.5 MHz | +25% |

**Fix:** Add explicit divider enum + validation
```c
typedef enum {
    HAL_SPI_BAUD_DIV_4 = 0,   // 25 MHz
    HAL_SPI_BAUD_DIV_8 = 1,   // 12.5 MHz
    HAL_SPI_BAUD_DIV_16 = 2,  // 6.25 MHz
    HAL_SPI_BAUD_DIV_32 = 3,  // 3.125 MHz
    HAL_SPI_BAUD_DIV_64 = 4,  // 1.56 MHz
    HAL_SPI_BAUD_DIV_128 = 5, // 781 kHz
    HAL_SPI_BAUD_DIV_256 = 6, // 390 kHz
    HAL_SPI_BAUD_DIV_512 = 7, // 195 kHz
    HAL_SPI_BAUD_DIV_1024 = 8, // 97.6 kHz
    HAL_SPI_BAUD_DIV_2048 = 9, // 48.8 kHz
} hal_spi_baud_div_t;

// Config option: either freq_hz OR baud_div (mutually exclusive)
```

### 1.2 Missing Timeout Support
**Problem:** All blocking calls wait forever on `SPI_wait_if_busy`, `TX_HOLD_EMPTY`, `RX_COMPLETE`. Hardware hang = infinite loop.

**Fix:** Add timeout parameter
```c
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms);
int hal_spi_write_read(..., uint32_t timeout_ms);
int hal_spi_write(..., uint32_t timeout_ms);
int hal_spi_read(..., uint32_t timeout_ms);
```

### 1.3 No Thread Safety
**Problem:** Per-bus state (`g_bus_config`, `g_bus_initialized`) not protected. FreeRTOS multi-task access = race condition.

**Fix:** Add mutex per bus
```c
// In spi.c
static StaticSemaphore_t g_bus_mutex[HAL_SPI_BUS_COUNT];
static SemaphoreHandle_t g_bus_mutex_handle[HAL_SPI_BUS_COUNT];

// In each public function:
xSemaphoreTake(g_bus_mutex_handle[bus], portMAX_DELAY);
// ... operation ...
xSemaphoreGive(g_bus_mutex_handle[bus]);
```

### 1.4 Missing Half-Duplex Optimization
**Problem:** `hal_spi_write` and `hal_spi_read` still do full-duplex (send dummy, read dummy). Waste cycles.

**Fix:** Use BSP's `SPI_write_tx_reg` / `SPI_read_rx_reg` for half-duplex when possible.

### 1.5 CS Mode Documentation Ambiguity
**Problem:** `HAL_SPI_CS_MODE_HARDWARE` name suggests "hardware auto-CS" but implementation is "manual CSAAT register control".

**Fix:** Rename enum for clarity:
```c
typedef enum {
    HAL_SPI_CS_MODE_CSAAT    = 0,  // Manual CSAAT register control (fixed CS0)
    HAL_SPI_CS_MODE_GPIO     = 1,  // Manual GPIO pin control
    HAL_SPI_CS_MODE_NONE     = 2,  // No CS (externally tied)
    // Future: HAL_SPI_CS_MODE_VARIABLE = 3,  // PS=1, PCS bits for SS0-SS3
} hal_spi_cs_mode_t;
```

### 1.6 Word Size 16-bit Unverified
**Problem:** `Dbits` field set for 16-bit but never tested. BSP only uses 8-bit.

**Fix:** Remove 16-bit support or mark experimental:
```c
// Only 8-bit supported for now
// if (config->word_size != 8) return -ENOTSUP;
```

### 1.7 Missing Features from BSP
| BSP Feature | HAL Status | Needed? |
|-------------|------------|---------|
| `SPI_check_overrun()` | ❌ Missing | Yes — data integrity |
| `SPI_intr_handler()` / `spi_register_isr()` | ❌ Missing | Future — async |
| Variable peripheral (PS=1, 4 CS) | ❌ Missing | Future — multi-device bus |
| Slave mode | ❌ Unknown | Probably not |

---

## 2. GPIO HAL — Issues

### 2.1 Missing Features
```c
// Current: init, set_dir, write, read, toggle, pulse_duration
// Missing from BSP:
int hal_gpio_set_interrupt(hal_gpio_pin_t pin, hal_gpio_int_edge_t edge, void (*isr)(void));
int hal_gpio_get_interrupt_status(hal_gpio_pin_t pin);
int hal_gpio_clear_interrupt(hal_gpio_pin_t pin);
// PWM output on GPIO (BSP has pwmc.c)
```

### 2.2 No Pin Mux Configuration
**Problem:** BSP has no pinmux API in gpio.h. GPIO pins may need alternate function selection (SPI, UART, I2C, PWM).

### 2.3 Direction Not Actually Set
```c
int hal_gpio_set_dir(hal_gpio_pin_t pin, hal_gpio_dir_t dir) {
    (void)dir;  // IGNORED!
    return 0;
}
```
BSP `gpio.h` has no direction function — assumed hardware handles it or not implemented.

---

## 3. UART HAL — Issues

### 3.1 Config Ignored
```c
int hal_uart_init(hal_uart_t uart, const hal_uart_config_t *config) {
    (void)config;  // IGNORED!
    // Hardcoded to UART0 at 115200
}
```
BSP `debug_uart.c` has fixed baud rate. No runtime config.

### 3.2 Read Blocking Without Timeout
```c
int hal_uart_read(hal_uart_t uart, uint8_t *data, uint32_t len, uint32_t timeout_ms) {
    (void)timeout_ms;  // IGNORED!
    // Busy-waits forever on rx_uart()
}
```

### 3.3 Only UART0 Supported
```c
if (uart != HAL_UART_0) return -ENOTSUP;
```

### 3.4 No Flow Control, Parity, Stop Bits
Config struct has them but BSP doesn't support.

---

## 4. Timer HAL — Issues

### 4.1 No FreeRTOS Integration
BSP has `timer.c` with ISR support. HAL `timer.c` only has `hal_timer_delay_ms`.

### 4.2 Missing Periodic/One-Shot Timers
```c
// Needed for FreeRTOS tickless idle, timeouts, etc.
int hal_timer_start_periodic(uint32_t period_us, void (*callback)(void));
int hal_timer_start_oneshot(uint32_t delay_us, void (*callback)(void));
int hal_timer_stop(void);
```

### 4.3 No High-Resolution Counter
BSP `timer.c` has 64-bit mtime at 0x20010580. HAL doesn't expose it.

---

## 5. Cross-Cutting Concerns

### 5.1 No Common Error Handling Pattern
| Module | Error Style |
|--------|-------------|
| SPI | `int` return, negative = errno |
| GPIO | `int` return, negative = errno |
| UART | `int` return, negative = errno |
| Timer | `int` return (only delay_ms) |

**Good** — consistent. But no centralized error strings or logging.

### 5.2 No Device Tree / Board Config
Board pin assignments hardcoded in `board.h`:
```c
#define BOARD_LED_RED_PIN    ((hal_gpio_pin_t)24)
#define BOARD_SPI_FLASH_BUS  HAL_SPI_BUS_0
```
Should be configurable per board variant.

### 5.3 No Power Management Hooks
BSP has clock gating, sleep modes. HAL has no `hal_pm_sleep`, `hal_pm_wakeup`.

### 5.4 No DMA Support
BSP has no DMA drivers. If hardware supports it, HAL should have abstract DMA API.

---

## 6. Priority Fix List

| Priority | Task | Effort |
|----------|------|--------|
| **P0** | SPI: Add timeout to all blocking calls | Medium |
| **P0** | SPI: Add mutex for thread safety | Medium |
| **P0** | SPI: Fix baud rate — add explicit divider enum | Low |
| **P1** | SPI: Add `hal_spi_check_overrun()` | Low |
| **P1** | GPIO: Implement direction setting or document limitation | Low |
| **P1** | UART: Implement timeout on read | Medium |
| **P1** | Timer: Add periodic/oneshot timer API | Medium |
| **P2** | SPI: Add interrupt/DMA support | High |
| **P2** | GPIO: Add interrupt API | Medium |
| **P2** | Board config: Device tree or Kconfig-style | High |
| **P3** | Power management HAL | High |

---

## 7. SPI HAL Refined Design (Proposed)

```c
// hal_spi.h — refined
typedef enum {
    HAL_SPI_BAUD_DIV_4   = 0,
    HAL_SPI_BAUD_DIV_8   = 1,
    HAL_SPI_BAUD_DIV_16  = 2,
    HAL_SPI_BAUD_DIV_32  = 3,
    HAL_SPI_BAUD_DIV_64  = 4,
    HAL_SPI_BAUD_DIV_128 = 5,
    HAL_SPI_BAUD_DIV_256 = 6,
    HAL_SPI_BAUD_DIV_512 = 7,
    HAL_SPI_BAUD_DIV_1024 = 8,
    HAL_SPI_BAUD_DIV_2048 = 9,
} hal_spi_baud_div_t;

typedef struct {
    hal_spi_mode_t       mode;
    hal_spi_bit_order_t  bit_order;
    hal_spi_cs_mode_t    cs_mode;
    union {
        uint8_t gpio_pin;
        hal_spi_cs_hw_t hw_cs;
    };
    // Frequency OR divider (one must be 0)
    uint32_t             freq_hz;
    hal_spi_baud_div_t   baud_div;
    uint8_t              word_size;  // 8 only for now
} hal_spi_config_t;

// Timeout on all blocking ops
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms);
int hal_spi_write_read(hal_spi_bus_t bus, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len, uint32_t timeout_ms);
int hal_spi_write(hal_spi_bus_t bus, const uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_spi_read(hal_spi_bus_t bus, uint8_t *data, uint32_t len, uint32_t timeout_ms);

// Overrun check
int hal_spi_check_overrun(hal_spi_bus_t bus);
bool hal_spi_is_busy(hal_spi_bus_t bus);
```

---

*Analysis complete. Ready for incremental implementation starting with P0 items.*