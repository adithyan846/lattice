# SPI HAL — Dual Build Implementation Complete

**Date:** 2026-11-03  
**Status:** ✅ Complete

---

## Summary

Implemented dual-build SPI HAL for THEJAS32 with two configurations:

### 1. Bare-Metal HAL (`spi_baremetal.c`)
- **Build**: `make`
- **RAM**: 68 KB
- **FreeRTOS**: ❌ Not required
- **Use case**: Bootloaders, standalone apps, bare-metal
- **Features**:
  - Manual CSAAT CS control per transaction
  - Configurable timeout (calibrated loop counter)
  - Explicit baud divider enum (`HAL_SPI_BAUD_DIV_4` to `HAL_SPI_BAUD_DIV_2048`)
  - Thread-safe not needed (single-threaded)
  - Overrun detection, busy status

### 2. FreeRTOS HAL (`spi_freertos.c`)
- **Build**: `make WITH_FREERTOS=1`
- **RAM**: 90 KB (+22 KB for mutexes)
- **FreeRTOS**: ✅ Required (`semphr.h`, `task.h`)
- **Use case**: Multi-task firmware with shared SPI bus
- **Features**:
  - Per-bus mutex (`xSemaphoreCreateMutex`)
  - RTOS-aware timeout (`pdMS_TO_TICKS`, `xTaskGetTickCount`)
  - Same API, same config struct
  - Same explicit baud divider enum

---

## Build Commands

```bash
# Bare-metal (default)
make clean && make

# FreeRTOS with mutex + timeout
make clean && make WITH_FREERTOS=1
```

---

## Files Created/Modified

| File | Purpose |
|------|---------|
| `hal/include/hal_spi.h` | Public API (both builds) |
| `platform/thejas32/spi_baremetal.c` | Bare-metal implementation |
| `platform/thejas32/spi_freertos.c` | FreeRTOS implementation |
| `Makefile` | Dual-build support with `WITH_FREERTOS` flag |

---

## HAL API (Common to Both)

```c
// Config
typedef enum { HAL_SPI_BAUD_DIV_4, ..., HAL_SPI_BAUD_DIV_2048 } hal_spi_baud_div_t;
typedef enum { HAL_SPI_CS_MODE_CSAAT, HAL_SPI_CS_MODE_GPIO, HAL_SPI_CS_MODE_NONE } hal_spi_cs_mode_t;

typedef struct {
    hal_spi_mode_t       mode;
    hal_spi_bit_order_t  bit_order;
    hal_spi_cs_mode_t    cs_mode;
    union { uint8_t gpio_pin; hal_spi_cs_hw_t hw_cs; };
    uint32_t             freq_hz;       // 0 = use baud_div
    hal_spi_baud_div_t   baud_div;      // Explicit divider
    uint8_t              word_size;     // 8 only
} hal_spi_config_t;

// Core API (all accept timeout_ms; 0 = infinite)
int hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config);
int hal_spi_deinit(hal_spi_bus_t bus);
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms);
int hal_spi_write_read(hal_spi_bus_t bus, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len, uint32_t timeout_ms);
int hal_spi_write(hal_spi_bus_t bus, const uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_spi_read(hal_spi_bus_t bus, uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_spi_set_cs(hal_spi_bus_t bus, bool assert);
int hal_spi_set_frequency(hal_spi_bus_t bus, uint32_t freq_hz);
int hal_spi_set_baud_div(hal_spi_bus_t bus, hal_spi_baud_div_t div);
int hal_spi_set_mode(hal_spi_bus_t bus, hal_spi_mode_t mode);
int hal_spi_set_bit_order(hal_spi_bus_t bus, hal_spi_bit_order_t order);
int hal_spi_check_overrun(hal_spi_bus_t bus);
bool hal_spi_is_busy(hal_spi_bus_t bus);
```

---

## Key Fixes from Debugging

| Issue | Root Cause | Fix |
|-------|------------|-----|
| Missing `hal_spi_write_read` | HAL had `transfer/write/read` only | Added `write_read` for register read pattern |
| Wrong CS mode (auto-hold) | CSAAT=1 in config | CSAAT=0, manual `SPI_set_CSAAT_pin(HIGH/LOW)` per transaction |
| Wrong peripheral type | PS=1 (variable) | PS=0 (fixed CS0) |
| Wrong baud rate | 1 MHz → divider 128 (781 kHz) | Explicit divider `HAL_SPI_BAUD_DIV_32` (3.125 MHz) |
| Per-byte busy wait | Inside loop | Once per transaction |
| Missing timeout | Infinite busy wait | `timeout_ms` parameter on all blocking calls |
| No thread safety | Global state | Per-bus FreeRTOS mutex |

---

## Memory Comparison

| Build | Text | Data | BSS | Total RAM |
|-------|------|------|-----|-----------|
| Bare-metal | 34 KB | 528 B | 33 KB | **68 KB** |
| FreeRTOS | 40 KB | 528 B | 50 KB | **90 KB** |

---

## Verification

Both builds complete successfully with only pre-existing BSP/FreeRTOS warnings (no new warnings in SPI HAL).

```bash
$ make clean && make
# Bare-metal: 68 KB RAM, no FreeRTOS

$ make clean && make WITH_FREERTOS=1
# FreeRTOS: 90 KB RAM, mutex + timeout
```

---

*Implementation complete. Ready for BMP280 driver integration on SPI2/SS0.*