# SPI HAL Design — Portable Abstraction Layer

**Project:** Lattice Flight Controller (THEJAS32 / RV32IM)
**Date:** 2026-11-03
**Stage:** 4 — HAL Design
**Status:** Complete

---

## 1. Design Goals & Constraints

| Goal | Constraint |
|------|------------|
| Portable across MCUs | No vendor types/registers in public API |
| Support THEJAS32 SPI fully | Map all verified hardware features |
| Support future platforms (STM32, ESP32, etc.) | Platform-specific code isolated in translator |
| Zero dynamic allocation | Static config, stack buffers only |
| FreeRTOS compatible | Blocking with timeout, thread-safe per-bus |
| Minimal overhead | Inlineable fast path for single-byte |

---

## 2. Feature Classification

Based on Stages 1-3 analysis (BSP + hardware + examples):

### 2.1 Essential for Initial Implementation (Must Have)

| Feature | Reason | THEJAS32 Support |
|---------|--------|------------------|
| **Bus instance selection** | Multiple SPI buses on board | 3 physical ports (0,1,2) |
| **Master mode** | Only mode needed for sensors/flash/radio | Verified |
| **SPI Modes 0-3** | Different devices require different modes | Verified (Mode 0 for radio/display, Mode 3 for flash) |
| **Configurable frequency** | Device-specific speed limits (3-25 MHz) | ÷4 to ÷2048 dividers |
| **MSB/LSB first** | Some devices need LSB | Verified (all examples use MSB) |
| **Chip select handling** | Multi-device buses, transaction framing | CSAAT bit + GPIO fallback |
| **Blocking transfer (full-duplex)** | Core SPI operation | Simulated via TX dummy + RX |
| **Blocking write (TX only)** | Command streaming, display data | `SPI_transmit` loop |
| **Blocking read (RX only)** | Register reads, status polling | `SPI_receive` loop |
| **Init/Deinit** | Lifecycle management | `SPI_init` + config |
| **Error/status codes** | Detect overrun, timeout, invalid config | Overrun bit + timeout wrapper |

### 2.2 Useful for Future Expansion (Should Have)

| Feature | Reason | THEJAS32 Support |
|---------|--------|------------------|
| **Timeout parameter** | Prevent infinite busy-wait | Not in BSP — HAL must add |
| **Word size 8/16-bit** | Some ADCs/DACs use 16-bit | Hardware has DBITS field (unverified) |
| **Variable peripheral mode** | Multi-CS without GPIO | PS=1 + PCS bits (API only) |
| **Interrupt-driven transfer** | High-throughput, low-CPU | API exists, no example |
| **DMA transfer** | Large buffers, zero-copy | Not supported |
| **Slave mode** | Bidirectional comms | Unknown hardware support |

### 2.3 Platform-Specific (Keep in Translator)

| Feature | Reason | Handling |
|---------|--------|----------|
| **CSAAT bit manipulation** | THEJAS32-specific CS hold mechanism | Translator manages internally |
| **Baud divider values** | ÷4, ÷8, ÷16... not Hz | Translator converts Hz → divider |
| **Fixed vs Variable peripheral** | THEJAS32 register concept | Translator chooses based on CS config |
| **Slave select bits (TxData 16-17)** | Undocumented THEJAS32 feature | Translator ignores unless needed |
| **IRQ mapping (SPI→PLIC)** | RV32-specific | Translator handles if interrupts added |
| **Global `gSPItransfer` state** | BSP thread-safety issue | Translator uses local config copy |

---

## 3. Public HAL Interface Design

### 3.1 Type Definitions

```c
// hal/spi/hal_spi.h

typedef enum {
    HAL_SPI_BUS_0 = 0,
    HAL_SPI_BUS_1 = 1,
    HAL_SPI_BUS_2 = 2,
    HAL_SPI_BUS_3 = 3,
    HAL_SPI_BUS_COUNT = 4
} hal_spi_bus_t;

typedef enum {
    HAL_SPI_MODE_0 = 0,  // CPOL=0, CPHA=0
    HAL_SPI_MODE_1 = 1,  // CPOL=0, CPHA=1
    HAL_SPI_MODE_2 = 2,  // CPOL=1, CPHA=0
    HAL_SPI_MODE_3 = 3,  // CPOL=1, CPHA=1
} hal_spi_mode_t;

typedef enum {
    HAL_SPI_BIT_ORDER_MSB = 0,
    HAL_SPI_BIT_ORDER_LSB = 1,
} hal_spi_bit_order_t;

typedef enum {
    HAL_SPI_CS_MODE_HARDWARE = 0,  // Controller manages CS (CSAAT)
    HAL_SPI_CS_MODE_GPIO     = 1,  // HAL manages CS via GPIO
    HAL_SPI_CS_MODE_NONE     = 2,  // No CS (single device, always selected)
} hal_spi_cs_mode_t;

typedef struct {
    hal_spi_mode_t       mode;          // SPI mode 0-3
    uint32_t             freq_hz;       // Target frequency in Hz
    hal_spi_bit_order_t  bit_order;     // MSB/LSB first
    hal_spi_cs_mode_t    cs_mode;       // How CS is managed
    uint8_t              cs_gpio_pin;   // GPIO pin for CS (if GPIO mode)
    uint8_t              word_size;     // Bits per word (8 or 16)
} hal_spi_config_t;

typedef enum {
    HAL_SPI_OK = 0,
    HAL_SPI_ERR_INVALID_ARG = -1,
    HAL_SPI_ERR_NOT_INITIALIZED = -2,
    HAL_SPI_ERR_BUSY = -3,
    HAL_SPI_ERR_TIMEOUT = -4,
    HAL_SPI_ERR_OVERRUN = -5,
    HAL_SPI_ERR_NOT_SUPPORTED = -6,
    HAL_SPI_ERR_HW = -7,
} hal_spi_status_t;
```

### 3.2 Function Signatures

```c
// Initialization
hal_spi_status_t hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config);
hal_spi_status_t hal_spi_deinit(hal_spi_bus_t bus);

// Configuration (runtime changes)
hal_spi_status_t hal_spi_set_frequency(hal_spi_bus_t bus, uint32_t freq_hz);
hal_spi_status_t hal_spi_set_mode(hal_spi_bus_t bus, hal_spi_mode_t mode);
hal_spi_status_t hal_spi_set_bit_order(hal_spi_bus_t bus, hal_spi_bit_order_t order);

// Core Transfer Operations
hal_spi_status_t hal_spi_transfer(hal_spi_bus_t bus,
                                   const uint8_t *tx, uint8_t *rx, uint32_t len,
                                   uint32_t timeout_ms);

hal_spi_status_t hal_spi_write(hal_spi_bus_t bus,
                                const uint8_t *data, uint32_t len,
                                uint32_t timeout_ms);

hal_spi_status_t hal_spi_read(hal_spi_bus_t bus,
                               uint8_t *data, uint32_t len,
                               uint32_t timeout_ms);

// Convenience: Single-byte (inlineable fast path)
static inline hal_spi_status_t hal_spi_transfer_byte(hal_spi_bus_t bus, uint8_t tx, uint8_t *rx);
static inline hal_spi_status_t hal_spi_write_byte(hal_spi_bus_t bus, uint8_t tx);
static inline hal_spi_status_t hal_spi_read_byte(hal_spi_bus_t bus, uint8_t *rx);

// Chip Select Control (for GPIO mode)
hal_spi_status_t hal_spi_cs_assert(hal_spi_bus_t bus);
hal_spi_status_t hal_spi_cs_deassert(hal_spi_bus_t bus);

// Status
hal_spi_status_t hal_spi_check_overrun(hal_spi_bus_t bus);
bool hal_spi_is_busy(hal_spi_bus_t bus);
```

---

## 4. HAL ↔ VEGA BSP Mapping Table

| HAL Operation | VEGA BSP Operation | Translation Required |
|---------------|-------------------|---------------------|
| `hal_spi_init(bus, config)` | `SPI_init(bus)` → `SPI_config(bus, ctrl)` → `SPI_set_baud(bus, div)` | 1. Map `freq_hz` → divider index (÷4..÷2048)<br>2. Build `SPIcntrlRegType` from `mode`, `bit_order`, `cs_mode`, `word_size`<br>3. Handle `cs_mode`: if GPIO → init GPIO pin, if HARDWARE → use CSAAT |
| `hal_spi_deinit(bus)` | (No direct equivalent) | 1. Reset Control register to defaults<br>2. Disable interrupts<br>3. If GPIO CS → deinit GPIO |
| `hal_spi_set_frequency(bus, hz)` | `SPI_set_baud(bus, div)` | 1. Calculate divider: `div = clk_freq / hz`<br>2. Clamp to valid divider index (0-9)<br>3. Apply via `SPI_set_baud` |
| `hal_spi_set_mode(bus, mode)` | `SPI_config(bus, ctrl)` | 1. Read current Control<br>2. Modify CPOL/CPHA bits (5:4)<br>3. Write back |
| `hal_spi_set_bit_order(bus, order)` | `SPI_config(bus, ctrl)` | 1. Read current Control<br>2. Modify DataOrder bit (3)<br>3. Write back |
| `hal_spi_transfer(bus, tx, rx, len, timeout)` | Loop: `SPI_transmit` + `SPI_receive` | 1. If `cs_mode == HARDWARE`: `SPI_set_CSAAT_pin(HIGH)`<br>2. If `cs_mode == GPIO`: `hal_gpio_write(cs_pin, LOW)`<br>3. For each byte: `SPI_transmit(tx[i])` → `rx[i] = SPI_receive()`<br>4. Add timeout checks in poll loops<br>5. Restore CS: CSAAT=LOW or GPIO=HIGH |
| `hal_spi_write(bus, data, len, timeout)` | Loop: `SPI_transmit` | 1. CS management (same as transfer)<br>2. For each byte: `SPI_transmit(data[i])`<br>3. Discard received dummy bytes<br>4. Timeout checks |
| `hal_spi_read(bus, data, len, timeout)` | Loop: `SPI_transmit(DUMMY)` + `SPI_receive` | 1. CS management<br>2. For each byte: `SPI_transmit(0x00)` → `data[i] = SPI_receive()`<br>3. Timeout checks |
| `hal_spi_cs_assert(bus)` | `SPI_set_CSAAT_pin(HIGH)` OR `GPIO_write(LOW)` | Depends on `cs_mode` |
| `hal_spi_cs_deassert(bus)` | `SPI_set_CSAAT_pin(LOW)` OR `GPIO_write(HIGH)` | Depends on `cs_mode` |
| `hal_spi_check_overrun(bus)` | `SPI_check_overrun(bus)` | Direct passthrough |
| `hal_spi_is_busy(bus)` | `SPI_wait_if_busy(bus)` (non-blocking check) | Read Status.BUSY bit directly |

---

## 5. Platform Translator Architecture

### 5.1 File Structure

```
hal/
└── spi/
    ├── hal_spi.h          # Public API (this design)
    └── hal_spi.c          # Dispatcher + common logic

platform/
└── thejas32/
    └── spi/
        ├── thejas32_spi.h  # Internal platform types
        └── thejas32_spi.c  # THEJAS32-specific implementation
```

### 5.2 Dispatcher Pattern (hal_spi.c)

```c
// hal/spi/hal_spi.c
#include "hal_spi.h"
#include "platform/thejas32/spi/thejas32_spi.h"

static const hal_spi_platform_ops_t *g_platform_ops = NULL;
static hal_spi_config_t g_bus_config[HAL_SPI_BUS_COUNT] = {0};
static bool g_bus_initialized[HAL_SPI_BUS_COUNT] = {false};

void hal_spi_register_platform(const hal_spi_platform_ops_t *ops) {
    g_platform_ops = ops;
}

hal_spi_status_t hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config) {
    if (!g_platform_ops || !g_platform_ops->init) return HAL_SPI_ERR_NOT_SUPPORTED;
    if (bus >= HAL_SPI_BUS_COUNT) return HAL_SPI_ERR_INVALID_ARG;
    if (!config) return HAL_SPI_ERR_INVALID_ARG;
    
    hal_spi_status_t ret = g_platform_ops->init(bus, config);
    if (ret == HAL_SPI_OK) {
        g_bus_config[bus] = *config;
        g_bus_initialized[bus] = true;
    }
    return ret;
}

hal_spi_status_t hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms) {
    if (!g_platform_ops || !g_platform_ops->transfer) return HAL_SPI_ERR_NOT_SUPPORTED;
    if (bus >= HAL_SPI_BUS_COUNT || !g_bus_initialized[bus]) return HAL_SPI_ERR_NOT_INITIALIZED;
    if (!tx && !rx) return HAL_SPI_ERR_INVALID_ARG;
    if (len == 0) return HAL_SPI_OK;
    
    return g_platform_ops->transfer(bus, tx, rx, len, timeout_ms);
}
// ... other functions dispatch similarly
```

### 5.3 Platform Operations Table

```c
// platform/thejas32/spi/thejas32_spi.h
#include "hal_spi.h"

typedef struct {
    hal_spi_status_t (*init)(hal_spi_bus_t bus, const hal_spi_config_t *config);
    hal_spi_status_t (*deinit)(hal_spi_bus_t bus);
    hal_spi_status_t (*set_frequency)(hal_spi_bus_t bus, uint32_t freq_hz);
    hal_spi_status_t (*set_mode)(hal_spi_bus_t bus, hal_spi_mode_t mode);
    hal_spi_status_t (*set_bit_order)(hal_spi_bus_t bus, hal_spi_bit_order_t order);
    hal_spi_status_t (*transfer)(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms);
    hal_spi_status_t (*write)(hal_spi_bus_t bus, const uint8_t *data, uint32_t len, uint32_t timeout_ms);
    hal_spi_status_t (*read)(hal_spi_bus_t bus, uint8_t *data, uint32_t len, uint32_t timeout_ms);
    hal_spi_status_t (*cs_assert)(hal_spi_bus_t bus);
    hal_spi_status_t (*cs_deassert)(hal_spi_bus_t bus);
    hal_spi_status_t (*check_overrun)(hal_spi_bus_t bus);
    bool (*is_busy)(hal_spi_bus_t bus);
} hal_spi_platform_ops_t;

extern const hal_spi_platform_ops_t thejas32_spi_ops;
```

### 5.4 THEJAS32 Implementation (thejas32_spi.c)

```c
// platform/thejas32/spi/thejas32_spi.c
#include "thejas32_spi.h"
#include "spi.h"          // VEGA BSP
#include "config.h"       // VEGA BSP
#include "gpio.h"         // VEGA BSP (for GPIO CS mode)
#include "hal_errno.h"

// SPI peripheral clock frequency (assumed from SYSTEM_FREQUENCY)
#define THEJAS32_SPI_CLK_HZ  100000000UL

// Map HAL bus → VEGA SPI instance
static uint8_t hal_bus_to_vega[HAL_SPI_BUS_COUNT] = {0, 1, 2, 3};

// Divider lookup: index = divider value for SPI_BAUD_CFD_x
static const uint32_t baud_dividers[] = {4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};

// Build SPIcntrlRegType from HAL config
static void build_control_reg(const hal_spi_config_t *cfg, SPIcntrlRegType *ctrl) {
    ctrl->Bits.PeriphChipSelect = 0;  // Always CS0 for fixed mode
    ctrl->Bits.PeriphType = 0;        // Fixed peripheral
    ctrl->Bits.DataOrder = (cfg->bit_order == HAL_SPI_BIT_ORDER_LSB) ? 1 : 0;
    ctrl->Bits.Mode = cfg->mode;      // 0-3 maps directly
    ctrl->Bits.SPIrxTxIntr = RX_TX_INTR_DIS;  // Polling only for now
    ctrl->Bits.CSAAT = (cfg->cs_mode == HAL_SPI_CS_MODE_HARDWARE) ? 1 : 0;
    ctrl->Bits.Dbits = (cfg->word_size == 16) ? 1 : 0;  // 0=8bit, 1=16bit (unverified)
    ctrl->Bits.Reserved = 0;
}

static uint8_t hz_to_divider_index(uint32_t freq_hz) {
    if (freq_hz == 0) return 2;  // Default ÷16
    uint32_t target_div = THEJAS32_SPI_CLK_HZ / freq_hz;
    
    // Find closest divider
    uint8_t best_idx = 0;
    uint32_t best_diff = UINT32_MAX;
    for (uint8_t i = 0; i < 10; i++) {
        uint32_t diff = (baud_dividers[i] > target_div) 
                        ? baud_dividers[i] - target_div 
                        : target_div - baud_dividers[i];
        if (diff < best_diff) {
            best_diff = diff;
            best_idx = i;
        }
    }
    return best_idx;
}

hal_spi_status_t thejas32_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *cfg) {
    uint8_t vega_spi = hal_bus_to_vega[bus];
    
    SPI_init(vega_spi);
    
    SPIcntrlRegType ctrl;
    build_control_reg(cfg, &ctrl);
    SPI_config(vega_spi, ctrl);
    
    uint8_t div_idx = hz_to_divider_index(cfg->freq_hz);
    SPI_set_baud(vega_spi, div_idx);
    
    // If GPIO CS mode, initialize GPIO pin
    if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_set_direction(cfg->cs_gpio_pin, GPIO_DIR_OUTPUT);
        GPIO_write_pin(cfg->cs_gpio_pin, 1);  // Deassert (active low)
    }
    
    return HAL_SPI_OK;
}

hal_spi_status_t thejas32_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms) {
    uint8_t vega_spi = hal_bus_to_vega[bus];
    const hal_spi_config_t *cfg = &g_bus_config[bus];
    
    // Assert CS
    if (cfg->cs_mode == HAL_SPI_CS_MODE_HARDWARE) {
        SPI_set_CSAAT_pin(vega_spi, HIGH);
    } else if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_write_pin(cfg->cs_gpio_pin, 0);
    }
    
    // Transfer loop with timeout
    uint32_t start = hal_timer_get_tick();  // Assume HAL timer available
    for (uint32_t i = 0; i < len; i++) {
        uint8_t tx_byte = tx ? tx[i] : 0x00;
        uint8_t rx_byte = 0;
        
        // Wait for not busy with timeout
        while (SPIreg(vega_spi).Status & SPI_BUSY_BIT) {
            if (hal_timer_elapsed_ms(start) > timeout_ms) {
                // Deassert CS on timeout
                if (cfg->cs_mode == HAL_SPI_CS_MODE_HARDWARE) SPI_set_CSAAT_pin(vega_spi, LOW);
                else if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) GPIO_write_pin(cfg->cs_gpio_pin, 1);
                return HAL_SPI_ERR_TIMEOUT;
            }
        }
        
        // Wait for TX hold empty
        while (!(SPIreg(vega_spi).Status & SPI_TX_HOLD_EMPTY_BIT)) {
            if (hal_timer_elapsed_ms(start) > timeout_ms) return HAL_SPI_ERR_TIMEOUT;
        }
        
        SPIreg(vega_spi).TxData = tx_byte;
        
        // Wait for RX complete
        while (!(SPIreg(vega_spi).Status & SPI_RX_COMPLETE_BIT)) {
            if (hal_timer_elapsed_ms(start) > timeout_ms) return HAL_SPI_ERR_TIMEOUT;
        }
        
        rx_byte = SPIreg(vega_spi).RxData & 0xFF;
        if (rx) rx[i] = rx_byte;
    }
    
    // Deassert CS
    if (cfg->cs_mode == HAL_SPI_CS_MODE_HARDWARE) {
        SPI_set_CSAAT_pin(vega_spi, LOW);
    } else if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) {
        GPIO_write_pin(cfg->cs_gpio_pin, 1);
    }
    
    return HAL_SPI_OK;
}

// ... other functions similar pattern
```

---

## 6. CS Management Strategy

### 6.1 Three CS Modes

| Mode | Use Case | Implementation |
|------|----------|----------------|
| **HARDWARE** (CSAAT) | Single device, high-speed, multi-byte transactions | `SPI_set_CSAAT_pin(HIGH)` before sequence, `LOW` after |
| **GPIO** | Multi-device bus, or device needs CS toggling per transaction | HAL manages GPIO pin; assert before, deassert after each HAL call |
| **NONE** | Single device, CS permanently wired low | No CS control |

### 6.2 Decision Logic in Translator

```c
// In thejas32_spi_transfer():
if (cfg->cs_mode == HAL_SPI_CS_MODE_HARDWARE) {
    // Use CSAAT - efficient for multi-byte
    SPI_set_CSAAT_pin(vega_spi, HIGH);
    // ... transfer loop ...
    SPI_set_CSAAT_pin(vega_spi, LOW);
} else if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO) {
    // Use GPIO - flexible, works for any device
    GPIO_write_pin(cfg->cs_gpio_pin, 0);
    // ... transfer loop ...
    GPIO_write_pin(cfg->cs_gpio_pin, 1);
}
// HAL_SPI_CS_MODE_NONE: no CS manipulation
```

---

## 7. Error Handling & Timeouts

### 7.1 Timeout Implementation

All blocking operations accept `timeout_ms`:
- **0** = no timeout (infinite wait — legacy behavior)
- **>0** = timeout in milliseconds using HAL timer

```c
// Polling with timeout pattern
uint32_t start = hal_timer_get_tick();
while (!(STATUS & READY_BIT)) {
    if (timeout_ms > 0 && hal_timer_elapsed_ms(start) > timeout_ms) {
        return HAL_SPI_ERR_TIMEOUT;
    }
}
```

### 7.2 Error Code Mapping

| BSP Condition | HAL Status |
|---------------|------------|
| Invalid bus/config | `HAL_SPI_ERR_INVALID_ARG` |
| Bus not initialized | `HAL_SPI_ERR_NOT_INITIALIZED` |
| Timeout in poll loop | `HAL_SPI_ERR_TIMEOUT` |
| Overrun bit set | `HAL_SPI_ERR_OVERRUN` |
| Feature not supported (e.g., 16-bit, DMA) | `HAL_SPI_ERR_NOT_SUPPORTED` |
| Hardware fault (bus hang) | `HAL_SPI_ERR_HW` |

---

## 8. Thread Safety

- **Per-bus locking:** Each SPI bus gets a `StaticSemaphore_t` mutex (FreeRTOS)
- **Init/Deinit:** Protected by global init mutex
- **Transfer:** Lock bus mutex for entire transaction (including CS assert/deassert)
- **Config changes:** Lock bus mutex

```c
// In hal_spi.c
static StaticSemaphore_t g_bus_mutex[HAL_SPI_BUS_COUNT];
static SemaphoreHandle_t g_bus_mutex_handle[HAL_SPI_BUS_COUNT];

// In hal_spi_transfer():
xSemaphoreTake(g_bus_mutex_handle[bus], portMAX_DELAY);
// ... transfer with CS management ...
xSemaphoreGive(g_bus_mutex_handle[bus]);
```

---

## 9. Configuration Validation

```c
static hal_spi_status_t validate_config(const hal_spi_config_t *cfg) {
    if (cfg->freq_hz == 0 || cfg->freq_hz > THEJAS32_SPI_CLK_HZ / 4)
        return HAL_SPI_ERR_INVALID_ARG;
    if (cfg->mode > HAL_SPI_MODE_3)
        return HAL_SPI_ERR_INVALID_ARG;
    if (cfg->word_size != 8 && cfg->word_size != 16)
        return HAL_SPI_ERR_INVALID_ARG;  // 16-bit unverified
    if (cfg->cs_mode == HAL_SPI_CS_MODE_GPIO && cfg->cs_gpio_pin > GPIO_MAX_PIN)
        return HAL_SPI_ERR_INVALID_ARG;
    return HAL_SPI_OK;
}
```

---

## 10. Comparison: HAL vs BSP API

| Aspect | VEGA BSP API | Lattice HAL API |
|--------|--------------|-----------------|
| **Frequency** | Divider index (0-9) | Hz (uint32_t) |
| **Mode** | Bitfield in control word | Enum `HAL_SPI_MODE_0..3` |
| **CS Control** | Manual CSAAT bit + slave select bits | `cs_mode` enum + GPIO pin |
| **Transfer** | Byte-by-byte functions | Buffer-based with length |
| **Timeout** | None (infinite) | Configurable ms |
| **Error Codes** | None (void/bool) | Enum `hal_spi_status_t` |
| **Thread Safety** | None (global state) | Per-bus mutex |
| **Portability** | THEJAS32 only | Platform-agnostic |

---

## 11. Files to Create/Modify

| File | Action |
|------|--------|
| `hal/spi/hal_spi.h` | Update with new design (this doc) |
| `hal/spi/hal_spi.c` | Implement dispatcher + common logic |
| `platform/thejas32/spi/thejas32_spi.h` | New: platform ops declaration |
| `platform/thejas32/spi/thejas32_spi.c` | New: THEJAS32 implementation |
| `platform/thejas32/CMakeLists.txt` | Add SPI platform files |
| `hal/CMakeLists.txt` | Add SPI HAL files |

---

## 12. Open Questions for Review

1. **SPI Clock Source:** We assume 100 MHz. Should HAL accept clock frequency in config?
2. **16-bit Word Size:** DBITS field exists but unverified. Expose in HAL or hide?
3. **Interrupt/DMA:** Add async API now or wait for need?
4. **Multi-CS (Variable Peripheral):** Support PS=1 + PCS bits for up to 4 devices per bus?
5. **Default Timeout:** What's a sensible default (e.g., 100ms)?
6. **Bus Sharing:** Multiple devices on same SPI bus with different configs — how to handle reconfiguration?

---

*Document generated as part of Stage 4 HAL Design for Lattice SPI HAL development.*