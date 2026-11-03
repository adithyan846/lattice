# SPI HAL Debugging Report — BMP280 on THEJAS32

**Date:** 2026-11-03  
**Status:** RESOLVED  
**Project:** Lattice Flight Controller (THEJAS32 / RV32IM)

---

## 1. Problem Summary

| Version | Status | Description |
|---------|--------|-------------|
| `main.bak` (HAL-based) | ❌ **Failed** | BMP280 initialization failed, no valid data returned |
| `main.c` (BSP-direct) | ✅ **Working** | BMP280 reads temperature/pressure correctly |

Both used identical BMP280 driver logic — only the SPI layer differed.

---

## 2. Root Cause Analysis

### 2.1 Missing Function (Critical)
**`hal_spi_write_read()`** was called by the driver but **did not exist** in the HAL.
- Called at `main.bak:54`: `return hal_spi_write_read(BARO_SPI_BUS, &cmd, 1, buf, len);`
- HAL only had `hal_spi_transfer`, `hal_spi_write`, `hal_spi_read`
- Would cause linker error or undefined behavior

### 2.2 Hardware CS Mode Misimplementation (Critical)
The HAL's "HARDWARE CS" mode was fundamentally wrong:

| Aspect | HAL (Broken) | BSP Direct (Working) | Why It Matters |
|--------|--------------|---------------------|----------------|
| **Control Register CSAAT** | Set to `1` (auto-hold) | Set to `0` (manual) | With CSAAT=1, hardware keeps CS asserted after each byte. Manual `SPI_set_CSAAT_pin(HIGH/LOW)` conflicts with hardware behavior. |
| **Peripheral Type (PS bit)** | `1` (Variable/PS=1) | `0` (Fixed/PS=0) | Variable mode uses PCS bits for SS0-SS3 selection. Fixed mode uses dedicated CS0 line. BMP280 needs simple CS0. |
| **PeriphChipSelect** | `hw_cs` (0-3) | `0` | In variable mode, this selects which SS line. In fixed mode, always CS0. |
| **CS Control Pattern** | Relied on CSAAT auto-hold | Manual `HIGH`/`LOW` per transaction | BMP280 register read/write requires CS deasserted between commands. |

### 2.3 Baud Rate Mismatch (High)
| Config | Value | Actual Frequency |
|--------|-------|------------------|
| HAL `freq_hz = 1000000` | Divider 128 | **781.25 kHz** |
| BSP `SPI_BAUD_CFD_32` | Divider 32 | **3.125 MHz** |

BMP280 datasheet recommends ≥1 MHz for reliable SPI. The HAL's divider calculation picked 128 (closest to 1 MHz target) but BSP example uses 32 (3.125 MHz).

### 2.4 Per-Byte `SPI_wait_if_busy` (Minor)
- HAL: Called inside loop for each byte
- BSP: Called once per transaction
- Adds unnecessary overhead, changes timing

---

## 3. Fix Applied

### 3.1 Added Missing Function
```c
// hal_spi.h
int hal_spi_write_read(hal_spi_bus_t bus, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len);

// platform/thejas32/spi.c
int hal_spi_write_read(...) {
    spi_assert_cs(...);
    // Write phase: send tx bytes, discard RX
    for (i=0; i<tx_len; i++) { ... }
    // Read phase: send 0xFF, capture RX
    for (i=0; i<rx_len; i++) { ... }
    spi_deassert_cs(...);
}
```

### 3.2 Fixed Hardware CS Mode (Core Fix)
**Changed `build_control_reg()` to match BSP pattern:**

```c
// BEFORE (Broken):
ctrl->Bits.CSAAT = 1;                    // Auto-hold CS
ctrl->Bits.PeriphType = 1;               // Variable peripheral
ctrl->Bits.PeriphChipSelect = hw_cs;     // Select SS0-SS3

// AFTER (Fixed - matches BSP direct):
ctrl->Bits.CSAAT = 0;                    // Manual control only
ctrl->Bits.PeriphType = 0;               // Fixed peripheral (CS0)
ctrl->Bits.PeriphChipSelect = 0;         // Always CS0
```

**CS control now explicit per transaction:**
```c
// In every transfer function:
spi_assert_cs()   → SPI_set_CSAAT_pin(HIGH)  // Assert CS
// ... loop: wait busy → TX → wait RX ...
spi_deassert_cs() → SPI_set_CSAAT_pin(LOW)   // Deassert CS
```

This exactly mirrors the BSP-direct pattern:
```c
SPI_set_CSAAT_pin(BARO_SPI_BUS, HIGH);  // Assert
// ... TX cmd + RX data ...
SPI_set_CSAAT_pin(BARO_SPI_BUS, LOW);   // Deassert
```

---

## 4. Verification

| Test | Result |
|------|--------|
| Build | ✅ Clean (no warnings on main.c) |
| Link | ✅ `hal_spi_write_read` resolved |
| Runtime (BSP-direct) | ✅ BMP280 reads temp/pressure correctly |
| Runtime (HAL) | ✅ Now matches BSP behavior |

---

## 5. Lessons Learned

1. **Never assume "hardware CS" means auto-hold** — for register-based devices, you need manual CS control per transaction
2. **Copy vendor example patterns exactly** — the BSP examples (AT25SF161, nRF24, RC522) all use: Fixed peripheral + CSAAT=0 + manual CS
3. **Validate against working reference** — BSP-direct version was the ground truth
4. **Baud rate matters** — divider calculation can pick suboptimal values; prefer explicit divider constants from BSP
5. **Missing API = silent failure** — `hal_spi_write_read` missing but compiler didn't catch it (implicit declaration)

---

## 6. Action Items for Future

- [ ] Add baud rate as explicit divider index option (not just Hz)
- [ ] Add transaction timeout support
- [ ] Document CS modes clearly in HAL header
- [ ] Consider adding `hal_spi_transfer_full_duplex` for symmetric TX/RX