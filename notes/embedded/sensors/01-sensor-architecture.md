# Sensor Architecture — Barometer Integration

**Date:** 2026-11-03  
**Project:** Lattice Flight Controller (THEJAS32 / RV32IM)  
**Status:** Complete

---

## 1. Architecture Overview

The sensor layer follows a **driver → interface → application** pattern:

```
┌─────────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER                         │
│  app/src/main.c                                                 │
│  - app_init()                                                   │
│  - app_update() → prints temperature, pressure, blinks LED      │
└──────────────────────────┬──────────────────────────────────────┘
                           │ #include "barometer.h"
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                      INTERFACE LAYER                             │
│  interfaces/barometer/                                          │
│  ├── barometer.h  — Public API                                   │
│  │   barometer_init()                                           │
│  │   barometer_update()                                         │
│  │   barometer_get_latest()                                     │
│  │   barometer_publish()                                        │
│  └── barometer.c  — Dispatch to device driver                   │
│      barometer_init()    → bmp280_init()                        │
│      barometer_update()  → bmp280_update()                      │
└──────────────────────────┬──────────────────────────────────────┘
                           │ #include "bmp280.h"
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                      DEVICE DRIVER LAYER                         │
│  devices/bmp280/                                                │
│  ├── bmp280.h  — Device API                                      │
│  │   bmp280_init()                                               │
│  │   bmp280_update()                                             │
│  └── bmp280.c  — SPI register communication via HAL            │
│      - SPI register read/write via hal_spi_write_read()         │
│      - Calibration coefficient decoding                         │
│      - BMP280 compensation algorithm (int64)                    │
│      - barometer_publish() with calibrated data                 │
└──────────────────────────┬──────────────────────────────────────┘
                           │ #include "hal_spi.h"
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                      HAL LAYER                                   │
│  hal/include/hal_spi.h                                          │
│  platform/thejas32/spi_baremetal.c  (or spi_freertos.c)        │
│      hal_spi_write_read(), hal_spi_write(), hal_spi_read()      │
└──────────────────────────┬──────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                      BSP LAYER                                   │
│  freertos-thejas32/.../bsp/drivers/spi/spi.c                   │
│      SPI_init(), SPI_config(), SPI_set_baud(),                 │
│      SPIreg().TxData/RxData, SPI_set_CSAAT_pin()               │
└──────────────────────────┬──────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                      HARDWARE                                    │
│  THEJAS32 SPI2 (SS0) → BMP280                                    │
│  Mode 3, 3.125 MHz (DIV_32), CSAAT manual CS                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. File Mapping

| Layer | Files | Responsibility |
|-------|-------|----------------|
| **Application** | `app/src/main.c` | Initialize HAL, call `barometer_init()`, loop `barometer_update()` + print |
| **Interface** | `interfaces/barometer/barometer.h` | Public API: `barometer_init()`, `barometer_update()`, `barometer_get_latest()`, `barometer_publish()` |
| **Interface** | `interfaces/barometer/barometer.c` | Dispatch to BMP280, store latest measurement |
| **Device** | `devices/bmp280/bmp280.h` | Device API: `bmp280_init()`, `bmp280_update()` |
| **Device** | `devices/bmp280/bmp280.c` | SPI register ops, calibration, compensation, publish via interface |
| **HAL** | `hal/include/hal_spi.h` | Portable SPI API: `hal_spi_write_read()`, `hal_spi_write()`, `hal_spi_read()` |
| **Platform** | `platform/thejas32/spi_baremetal.c` / `spi_freertos.c` | THEJAS32 implementation using BSP |
| **BSP** | `freertos-thejas32/.../bsp/drivers/spi/spi.c` | VEGA SDK SPI driver |
| **Hardware** | THEJAS32 SPI2 | Physical SPI controller, SS0 pin |

---

## 3. Data Flow

```
main.c
  app_update()
    │
    ▼
barometer_update()  (interfaces/barometer/barometer.c)
    │
    ├── Calls bmp280_update()  (devices/bmp280/bmp280.c)
    │     │
    │     ├── Triggers forced-mode measurement
    │     ├── Reads raw ADC (pressure + temperature) via SPI
    │     ├── Applies compensation algorithm (int64)
    │     └── Creates barometer_data_t with calibrated values
    │
    ▼
barometer_publish(&data)  (interfaces/barometer/barometer.c)
    │
    ├── Stores in static latest_data
    │
    ▼
barometer_get_latest(&data)  (main.c)
    │
    ▼
printf temperature_c, pressure_pa
```

---

## 4. Build Integration

### Makefile Additions

```makefile
# Added to C_SOURCES
$(ROOT)/interfaces/barometer/barometer.c \
$(ROOT)/devices/bmp280/bmp280.c \

# Added to LATTICE_INCLUDES
-I$(ROOT)/interfaces/barometer \
-I$(ROOT)/devices/bmp280 \
```

### Compile Commands

```bash
# Bare-metal (default)
make clean && make

# FreeRTOS with mutex + timeout
make clean && make WITH_FREERTOS=1

# Flash to board
make flash
```

---

## 5. Configuration

### Barometer SPI Config (main.c)

```c
#define BARO_SPI_BUS            HAL_SPI_BUS_2   // SPI2
#define BARO_CS_MODE            HAL_SPI_CS_MODE_CSAAT  // Manual CSAAT
#define BARO_HW_CS              HAL_SPI_CS_HW_0        // SS0
#define BARO_BAUD_DIV           HAL_SPI_BAUD_DIV_32    // 3.125 MHz
#define BARO_TIMEOUT_MS         100
```

### BMP280 Sensor Config (bmp280.h)

```c
#define BMP280_OSRS_T           0x02  // Temperature oversampling ×2
#define BMP280_OSRS_P           0x05  // Pressure oversampling ×16
#define BMP280_IIR_FILTER       0x04  // IIR filter coefficient 16
```

---

## 6. Extending for New Sensors

To add a new sensor (e.g., MPU6000 IMU):

```
1. Create interface:
   interfaces/imu/
   ├── imu.h          # Public API
   └── imu.c          # Dispatch to device

2. Create device driver:
   devices/mpu6000/
   ├── mpu6000.h      # Device API
   └── mpu6000.c      # SPI/I2C register ops, calibration

3. Add to Makefile:
   C_SOURCES += \
     $(ROOT)/interfaces/imu/imu.c \
     $(ROOT)/devices/mpu6000/mpu6000.c
   LATTICE_INCLUDES += \
     -I$(ROOT)/interfaces/imu \
     -I$(ROOT)/devices/mpu6000

4. Update application:
   #include "imu.h"
   imu_init();
   imu_update();
   imu_get_latest(&data);
```

---

## 7. Notes

- **No circular dependencies**: Application → Interface → Device → HAL (unidirectional)
- **Interface owns data storage**: `latest_data` in barometer.c, not in device
- **Device is stateless**: All state in `bmp280_dev_t` static, not in interface
- **Dual HAL support**: Works with both `spi_baremetal.c` and `spi_freertos.c`
- **Hardware CS**: Uses CSAAT register manual control per transaction

---

*Documentation generated as part of Lattice sensor architecture documentation.*