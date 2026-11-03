# FreeRTOS Integration in SPI HAL — Design Rationale

**Date:** 2026-11-03  
**Context:** SPI HAL refinement for Lattice Flight Controller (THEJAS32 / RV32IM)

---

## 1. Why FreeRTOS in HAL?

### Problem: Multi-Task SPI Bus Sharing
In a FreeRTOS application, multiple tasks may access the same SPI bus concurrently:

```
Task A (Barometer)          Task B (Flash)           Task C (Radio)
     │                           │                        │
     └────────── SPI2 ───────────┴───────────────────────┘
                    │
              Single SPI Controller
```

Without synchronization:
- Task A starts transaction → CS asserted
- Task B preempts → CS deasserted, new transaction starts
- Task A resumes → reads garbage data, BMP280 confused

### Solution: Mutex Per Bus
```c
// In hal_spi_init()
g_bus_mutex[bus] = xSemaphoreCreateMutex();

// In every public function:
spi_lock(bus, timeout_ms);      // xSemaphoreTake()
... transaction ...
spi_unlock(bus);                // xSemaphoreGive()
```

### What FreeRTOS Provides
| RTOS Primitive | HAL Usage |
|----------------|-----------|
| `xSemaphoreCreateMutex()` | Per-bus mutex creation |
| `xSemaphoreTake()` | Block until bus available (with timeout) |
| `xSemaphoreGive()` | Release bus for other tasks |
| `pdMS_TO_TICKS()` | Convert ms → RTOS ticks |
| `xTaskGetTickCount()` | Track elapsed time for polling timeouts |
| `portMAX_DELAY` | Infinite wait option |

---

## 2. Cost of FreeRTOS Dependency

| Aspect | Impact |
|--------|--------|
| **Code size** | +~4 KB (mutex, scheduler hooks) |
| **RAM** | +~4 KB (StaticSemaphore_t × 4 buses) |
| **Link dependency** | Requires FreeRTOS library |
| **Build config** | Needs FreeRTOSConfig.h, semphr.h, task.h |
| **Bare-metal** | Cannot compile without FreeRTOS |

---

## 3. Dual-BUILD Strategy

### Build Modes
| Mode | Command | HAL Used | Use Case |
|------|---------|----------|----------|
| **Bare-metal** | `make` | `hal_spi.c` (no RTOS) | Bootloader, standalone apps |
| **FreeRTOS** | `make WITH_FREERTOS=1` | `hal_spi_freertos.c` | Main firmware with tasks |

### File Structure
```
hal/
└── spi/
    ├── hal_spi.h              # Public API (identical)
    ├── hal_spi_baremetal.c    # No RTOS, spinlocks only
    └── hal_spi_freertos.c     # FreeRTOS mutex + timeouts

platform/thejas32/spi/
    ├── spi_baremetal.c        # THEJAS32 impl, no RTOS
    └── spi_freertos.c         # THEJAS32 impl, FreeRTOS
```

### Public API (Unchanged)
```c
// hal_spi.h — SAME for both builds
int hal_spi_init(hal_spi_bus_t bus, const hal_spi_config_t *config);
int hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len, uint32_t timeout_ms);
int hal_spi_write_read(...);
int hal_spi_write(...);
int hal_spi_read(...);
// ... all functions accept timeout_ms
```

---

## 4. Bare-Metal Implementation Notes

### Mutex Replacement
```c
// baremetal: simple spinlock (or nothing for single-threaded)
static volatile int g_bus_locked[HAL_SPI_BUS_COUNT] = {0};

static inline int spi_lock(hal_spi_bus_t bus, uint32_t timeout_ms) {
    // Simple atomic test-and-set or just disable interrupts
    while (__sync_lock_test_and_set(&g_bus_locked[bus], 1)) {
        if (timeout_ms == 0) continue;
        // Simple spin timeout
        if (--timeout_ms == 0) return -ETIMEDOUT;
        for (volatile int i=0; i<1000; i++);
    }
    return 0;
}

static inline void spi_unlock(hal_spi_bus_t bus) {
    __sync_lock_release(&g_bus_locked[bus]);
}
```

### Timeout Replacement
```c
// baremetal: cycle counter or simple loop counter
static inline int spi_wait_status(uint8_t vega_spi, uint32_t mask, int want_set, 
                                   uint32_t timeout_ms, uint32_t start_tick) {
    uint32_t loops = timeout_ms * 10000;  // Calibrated
    while (1) {
        uint32_t status = SPIreg(vega_spi).Status;
        if (((status & mask) != 0U) == (want_set != 0)) return 0;
        if (loops-- == 0) return -ETIMEDOUT;
    }
}
```

---

## 5. Build System Integration

```makefile
# In Makefile
ifeq ($(WITH_FREERTOS),1)
    SPI_HAL_SRC = platform/thejas32/spi_freertos.c
    CFLAGS += -DWITH_FREERTOS
else
    SPI_HAL_SRC = platform/thejas32/spi_baremetal.c
endif

$(ELF): $(SPI_HAL_SRC) ...
```

---

## 6. Recommendation

**For Lattice Flight Controller:**
- Main firmware → `make WITH_FREERTOS=1` (FreeRTOS HAL)
- Bootloader/early init → `make` (bare-metal HAL)

This keeps the HAL portable while enabling RTOS safety when needed.

---

*Documentation complete. Ready to implement dual-build.*