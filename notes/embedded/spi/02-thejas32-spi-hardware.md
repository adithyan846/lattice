# THEJAS32 SPI Hardware Analysis

**Project:** Lattice Flight Controller (THEJAS32 / RV32IM)
**Date:** 2026-11-03
**Stage:** 2 — SPI Hardware Understanding
**Status:** Complete

---

## 1. SPI Controller Architecture Overview

Based on register map analysis from `spi.h:111-124`, the THEJAS32 SPI controller is a **simple synchronous serial peripheral** with the following characteristics:

| Characteristic | Value | Source |
|----------------|-------|--------|
| **Instances** | 4 (SPI 0–3) | `spi.h:33-36` |
| **Register stride** | 0x100 (256 bytes) per instance | `spi.h:135` |
| **Bus interface** | Memory-mapped (32-bit aligned) | `config.h:26` |
| **Data width** | 8-bit only (verified) | `spi.h:58`, `at25sf161.c:55` |
| **FIFO** | Single-entry hold register (no deep FIFO) | Status bits imply 1-deep |
| **Operating modes** | Master only (verified) | No slave API exists |
| **Transfer modes** | Polling (verified), Interrupt (API only) | `spi.c` implementation |
| **DMA** | Not supported | No DMA registers/APIs |

**Memory Map:**
```
SPI 0: 0x10000600 – 0x100006FF
SPI 1: 0x10000700 – 0x100007FF
SPI 2: 0x10200100 – 0x102001FF
SPI 3: 0x10200200 – 0x102002FF
```

**Note:** SPI 2/3 share base region `0x10200100` with QSPI (`qspi.h:88`). This suggests SPI 2/3 may be the same physical controller as QSPI, or they share a peripheral cluster.

---

## 2. Register Layout Analysis

### Register Map (Offsets from Instance Base)

| Offset | Register | Width | Access | Description |
|--------|----------|-------|--------|-------------|
| 0x00 | **Control** | 16-bit | R/W | Mode, CS, bit order, interrupts, data width, CSAAT |
| 0x04 | **Status** | 32-bit | R | Busy, overrun, RX complete, TX empty, interrupt flags |
| 0x08 | **Baudrate** | 32-bit | R/W | Clock divider (value << 4) |
| 0x0C | **TxData** | 32-bit | W | Transmit data + slave select (bits 16-17) |
| 0x10 | **RxData** | 32-bit | R | Receive data + slave select (bits 16-17) |

**Control Register (0x00) — 16-bit Bitfield Layout:**

```
Bit:  15 14 13 | 12 11 10 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 0
      Reserved  |  DBITS     |CSA|SPT|SPR|CPO|CPH|LSB| PS|PCS1 PCS0
                |  [12:9]    |AT |IE |IE |L  |A  |1st|   |
```

| Bits | Field | Reset | Description |
|------|-------|-------|-------------|
| 15:13 | Reserved | 0 | Read/Write, write as 0 |
| 12:9 | **DBITS** | 0 | Data bits per transfer: 0 = 8-bit (only value used) |
| 8 | **CSAAT** | 0 | Chip Select Active After Transfer: 1 = hold CS low after transfer |
| 7 | **SPTIE** | 0 | SPI Transmit Interrupt Enable: 1 = interrupt on TX hold empty |
| 6 | **SPRIE** | 0 | SPI Receive Interrupt Enable: 1 = interrupt on RX complete |
| 5 | **CPOL** | 0 | Clock Polarity: 0 = idle low, 1 = idle high |
| 4 | **CPHA** | 0 | Clock Phase: 0 = sample leading, 1 = sample trailing |
| 3 | **LSB/MSB** | 0 | 0 = MSB first, 1 = LSB first |
| 2 | **PS** | 0 | Peripheral Select: 0 = Fixed (CS0), 1 = Variable (PCS bits) |
| 1:0 | **PCS** | 00 | Peripheral Chip Select (active when PS=1) |

---

## 3. Control Register Deep Dive

### 3.1 SPI Mode Encoding (CPOL/CPHA)

From `spi.c:100-112` documentation:

| SPI Mode | CPOL (bit 5) | CPHA (bit 4) | Clock Idle | Sample Edge | Setup Edge |
|----------|--------------|--------------|------------|-------------|------------|
| **0** | 0 | 0 | Low | Rising (leading) | Falling (trailing) |
| **1** | 0 | 1 | Low | Falling (trailing) | Rising (leading) |
| **2** | 1 | 0 | High | Falling (leading) | Rising (trailing) |
| **3** | 1 | 1 | High | Rising (trailing) | Falling (leading) |

**Electrical Behavior:**
- **Mode 0 (most common):** SCK idles low. Data launched on falling edge, sampled on rising edge.
- **Mode 3 (used by AT25SF161):** SCK idles high. Data launched on rising edge, sampled on falling edge.

**BSP Default:** `SPI_init()` sets Mode 3 (`gSPItransfer.Bits.Mode = 3` at `spi.c:61`).

### 3.2 Chip Select Generation

Two modes controlled by **PS bit (bit 2)**:

#### Fixed Peripheral Mode (PS=0) — **BSP Default**
- Single dedicated CS line (CS0)
- CS asserted automatically during transfer
- `SPI_init()` sets `PeriphType = 0` and `PeriphChipSelect = 0`
- No manual CS control needed for single-device buses

#### Variable Peripheral Mode (PS=1)
- Up to 4 chip selects via PCS1:PCS0 bits
- Encoding (from `spi.c:120-127`):
  | PCS1 | PCS0 | Active CS Line |
  |------|------|----------------|
  | 0 | 0 | CS0 (1110) |
  | 0 | 1 | CS1 (1101) |
  | 1 | 0 | CS2 (1011) |
  | 1 | 1 | CS3 (0111) |
- **Note:** CS lines are active-low (0 = asserted)

### 3.3 CSAAT Bit (Bit 8) — Critical for Multi-Byte Transfers

**Hardware Behavior:**
- **CSAAT = 0:** CS deasserts after each 8-bit transfer (between bytes)
- **CSAAT = 1:** CS remains asserted (active low) across multiple transfers

**BSP Usage (from `at25sf161.c`):**
```c
SPI_set_CSAAT_pin(spi, HIGH);   // Set CSAAT=1 before multi-byte sequence
// ... multiple SPI_transmit() calls ...
SPI_set_CSAAT_pin(spi, LOW);    // Set CSAAT=0 after sequence
```

**Electrical Implication:** Without CSAAT, the flash device would see CS toggle between each command/address/data byte, aborting the transaction. Most SPI flash/EEPROM devices require CS held low for entire command sequence.

### 3.4 Data Order (Bit 3)

| Value | Meaning | BSP Default |
|-------|---------|-------------|
| 0 (MSB) | Most Significant Bit first | **Default** (`MSB` at `spi.h:47`) |
| 1 (LSB) | Least Significant Bit first | Available via `SPI_config()` |

**BSP Default:** MSB first (`gSPItransfer.Bits.DataOrder = MSB` at `spi.c:62`).

### 3.5 Data Width — DBITS Field (Bits 12:9)

| Value | Bits/Transfer | Verified? |
|-------|---------------|-----------|
| 0 | 8-bit | **Yes** — only value used in BSP |
| 1–15 | 9–24 bit? | **No** — no examples, unclear encoding |

**Critical Observation:** The BSP **only uses 8-bit transfers** (`Dbits = 0`). The field exists in hardware but:
- No example uses other widths
- `SPI_transmit`/`SPI_receive` take/return `US` (16-bit) but only low 8 bits used
- TxData/RxData are 32-bit registers but only 8-bit data observed

**Unverified:** Whether hardware actually supports >8-bit transfers or if DBITS is a vestigial field.

---

## 4. Status Register Deep Dive

### Status Register (0x04) — 32-bit

```
Bit:  7        6           5         4      3           2
      TX_HOLD_ RX_COMPLETE OVERRUN   BUSY   TX_INTR     RX_INTR
      _EMPTY   _BIT        _BIT      _BIT   _STATUS     _STATUS
```

| Bit | Macro | Name | Meaning |
|-----|-------|------|---------|
| 7 | `SPI_TX_HOLD_EMPTY_BIT` | TX Hold Empty | 1 = TX hold register empty, ready for new data |
| 6 | `SPI_RX_COMPLETE_BIT` | RX Complete | 1 = RX shift register loaded, data ready in RxData |
| 5 | `SPI_OVERR_BIT` | Overrun | 1 = New data received before previous read (data lost) |
| 4 | `SPI_BUSY_BIT` | Busy | 1 = SPI controller actively shifting |
| 3 | `SPI_TX_INT_STATUS_BIT` | TX Intr Status | 1 = TX interrupt condition met |
| 2 | `SPI_RX_INT_STATUS_BIT` | RX Intr Status | 1 = RX interrupt condition met |

### Hardware State Machine (Inferred from Status Bits)

```
IDLE → (write TxData) → SHIFTING → (shift complete) → RX_COMPLETE + TX_HOLD_EMPTY
                           ↓
                      BUSY=1
                           ↓
                    (if RX not read) → OVERRUN=1
```

**Key Observations:**
1. **Single-buffered (no FIFO):** TX_HOLD_EMPTY and RX_COMPLETE are mutually exclusive during transfer
2. **Overrun detection:** Hardware sets OVERRUN if RxData not read before next transfer completes
3. **Busy flag:** Asserted during entire shift operation (8 clock cycles for 8-bit)

---

## 5. Clock Generation & Baud Rate

### Baudrate Register (0x08)

```c
SPIreg(spi_number).Baudrate = (divider_value << 4);  // spi.c:170
```

### Divider Values (from `spi.h:79-88`)

| Macro | Register Value | Divider | @ 100 MHz System Clock |
|-------|----------------|---------|------------------------|
| `SPI_BAUD_CFD_4` | 0 | ÷4 | **25 MHz** |
| `SPI_BAUD_CFD_8` | 1 | ÷8 | 12.5 MHz |
| `SPI_BAUD_CFD_16` | 2 | ÷16 | 6.25 MHz |
| `SPI_BAUD_CFD_32` | 3 | ÷32 | 3.125 MHz |
| `SPI_BAUD_CFD_64` | 4 | ÷64 | 1.5625 MHz |
| `SPI_BAUD_CFD_128` | 5 | ÷128 | 781.25 kHz |
| `SPI_BAUD_CFD_256` | 6 | ÷256 | 390.625 kHz |
| `SPI_BAUD_CFD_512` | 7 | ÷512 | 195.3125 kHz |
| `SPI_BAUD_CFD_1024` | 8 | ÷1024 | 97.65625 kHz |
| `SPI_BAUD_CFD_2048` | 9 | ÷2048 | 48.828125 kHz |

### Clock Source Question

**Critical Unknown:** What is the SPI peripheral clock source?
- `config.h:23` defines `SYSTEM_FREQUENCY 100000000` (100 MHz)
- But is this the **CPU clock** or **peripheral bus clock**?
- The baud divider divides *some* clock — could be system clock, could be divided peripheral clock
- **No documentation found** in BSP confirming the source clock

**BSP Default:** `SPI_init()` uses `SPI_BAUD_CFD_16` (÷16 = 6.25 MHz @ 100 MHz)
**Flash Example:** `at25sf161.c:60` uses `SPI_BAUD_CFD_4` (÷4 = 25 MHz @ 100 MHz)

---

## 6. Data Transfer Mechanism

### 6.1 Transmit Path (Blocking Polling)

From `SPI_transmit()` at `spi.c:271-280`:

```c
void SPI_transmit(UC spi_number, US bData) {
    SPI_wait_if_busy(spi_number);           // 1. Wait for BUSY=0
    while (!(SPIreg(spi_number).Status & SPI_TX_HOLD_EMPTY_BIT))
        ;                                   // 2. Wait for TX_HOLD_EMPTY=1
    SPIreg(spi_number).TxData = bData;      // 3. Write data (starts transfer)
    __asm__ __volatile__ ("fence");
}
```

**Hardware Sequence:**
1. Write to TxData loads **TX hold register**
2. If shift register empty, data moves to shift register immediately → TX_HOLD_EMPTY=1, BUSY=1
3. Controller shifts 8 bits on SCK
4. On completion: BUSY=0, RX_COMPLETE=1, TX_HOLD_EMPTY=1

### 6.2 Receive Path (Blocking Polling)

From `SPI_receive()` at `spi.c:253-261`:

```c
US SPI_receive(UC spi_number) {
    US bRxData;
    UC status = 0;
    while (!((SPIreg(spi_number).Status) & SPI_RX_COMPLETE_BIT))
        ;                                   // Wait for RX_COMPLETE=1
    bRxData = SPIreg(spi_number).RxData;    // Read data
    return bRxData;
}
```

**Critical Design:** SPI is **full-duplex** hardware — every transmit generates a receive. The BSP API separates them:
- `SPI_transmit()` — writes TxData, ignores RxData
- `SPI_receive()` — reads RxData (must have been generated by prior transmit)

### 6.3 Full-Duplex Usage Pattern (from `at25sf161.c:94-100`)

```c
while (length) {
    SPI_transmit(currentSpi, DUMMY_DATA);   // Transmit dummy (0x00)
    *readBuf = SPI_receive(currentSpi);     // Read actual data
    readBuf++;
    length--;
}
```

**Electrical Reality:** Each loop iteration = one 8-bit full-duplex frame:
- MOSI sends `DUMMY_DATA` (0x00)
- MISO receives flash data
- SCK toggles 8 times
- CS held low (CSAAT=1)

---

## 7. Chip Select Electrical Behavior

### 7.1 Fixed Peripheral Mode (PS=0) — Default

```
CS0 waveform during SPI_transmit() with CSAAT=0:
     _______       _______       _______
____/       \_____/       \_____/       \____
    Byte 1      Byte 2      Byte 3

CS0 waveform with CSAAT=1:
____________________________________________
    Byte 1      Byte 2      Byte 3
```

**Hardware Behavior:** In fixed mode, CS0 asserts automatically when TxData written, deasserts after shift completes — **unless CSAAT=1**.

### 7.2 Variable Peripheral Mode (PS=1)

- CS lines selected by PCS1:PCS0 bits in Control register
- CS assertion timing same as fixed mode
- Allows multi-device bus without GPIO toggling

### 7.3 Slave Select in TxData/RxData Registers (Bits 16-17)

From `SPI_set_slave_select()` at `spi.c:198-205` and `SPI_get_slave_select()` at `spi.c:182-188`:

```c
// TxData bits 16-17 control something CS-related
write_data = read_data | (slave_sel_val << 16);
SPIreg(spi_number).TxData = write_data;

// RxData bits 16-17 readable
read_data = SPIreg(spi_number).RxData;
read_data = ((read_data & 0x30000) >> 16);
```

**Hypothesis:** These bits may control CS in variable peripheral mode, or reflect CS pin state. **Not clearly documented in BSP.**

---

## 8. Interrupt Architecture

### 8.1 Interrupt Sources (2 per SPI instance)

| Interrupt | Status Bit | Control Bit | Condition |
|-----------|------------|-------------|-----------|
| **RX Interrupt** | Bit 2 (RX_INTR_STATUS) | Bit 6 (SPRIE) | RX_COMPLETE=1 (data ready in RxData) |
| **TX Interrupt** | Bit 3 (TX_INTR_STATUS) | Bit 7 (SPTIE) | TX_HOLD_EMPTY=1 (ready for next byte) |

### 8.2 Interrupt Registration (from `spi.c:360-381`)

```c
void spi_register_isr(UC spi_number, void (*spi_isr)()) {
    UC irq_no;
    #if __riscv_xlen == 64
        // SPI 0=6, 1=7, 2=52, 3=53
    #else
        // SPI 0=3, 1=4, 2=23
    #endif
    interrupt_enable(irq_no);
    irq_register_handler(irq_no, spi_isr);
}
```

### 8.3 Interrupt Status Reading (from `spi.c:390-410`)

```c
UC spi_intr_status(UC spi_number) {
    UC status = SPIreg(spi_number).Status;
    if (status & (1 << 2))  return SPI_RX_INTR;  // Bit 2
    if (status & (1 << 3))  return SPI_TX_INTR;  // Bit 3
}
```

**Key Limitation:** Single interrupt vector per SPI instance — handler must read status to determine RX vs TX.

### 8.4 Verified Status

| Feature | Verified? | Evidence |
|---------|-----------|----------|
| Interrupt API exists | **Yes** | `spi_register_isr`, `SPI_set_intr`, `spi_intr_status` |
| Interrupt used in example | **No** | `at25sf161.c` uses polling only |
| IRQ mapping correct | **Partial** | Code exists, no test evidence |
| Interrupt latency | **Unknown** | No measurements |

---

## 9. FIFO / Buffer Architecture

### What We Can Infer

| Indicator | Observation | Inference |
|-----------|-------------|-----------|
| TX_HOLD_EMPTY bit | Single bit, not a count | 1-deep TX hold register |
| RX_COMPLETE bit | Single bit, not a count | 1-deep RX buffer |
| No FIFO level bits | None in Status register | No programmable FIFO thresholds |
| No DMA | No DMA registers | No DMA support |

**Conclusion:** **Single-buffered (double-buffered at most)** — one hold register + one shift register per direction. No FIFO.

**Implication:** At high baud rates, CPU must service each byte within 1 frame time (e.g., 320 ns @ 25 MHz). Polling loop in `SPI_transmit`/`SPI_receive` handles this by busy-waiting.

---

## 10. Verified vs. Unverified Hardware Capabilities

| Capability | Hardware Support | BSP Implemented | Example Verified | Notes |
|------------|------------------|-----------------|------------------|-------|
| 4 SPI instances | Likely | Yes | SPI 0 primarily | RV32 IRQs only for 0-2; user confirms 3 physical ports |
| Master mode | Yes | Yes | Yes | Only mode seen across all examples |
| Slave mode | Unknown | No | No | No slave APIs/registers found |
| SPI Modes 0-3 | Yes (CPOL/CPHA bits) | Yes | **Modes 0 & 3** | All 4 encodings in register; Mode 0 for radios/displays, Mode 3 for flash |
| 8-bit transfers | Yes | Yes | Yes | Only width used in all examples |
| 9-16 bit transfers | Maybe (DBITS field) | No | No | DBITS field exists, unused |
| MSB/LSB first | Yes | Yes | MSB only | Bit 3 in Control |
| Fixed CS (CS0) | Yes | Yes | Yes | PS=0 default, all examples use this |
| Variable CS (4x) | Yes (PCS bits) | API only | No | PS=1 supported in register, no example uses it |
| CSAAT (hold CS) | Yes | Yes | **Yes (critical)** | Used by ALL multi-byte examples (flash, radio, RFID, displays) |
| Polling transfer | Yes | Yes | Yes | Primary mode in all examples |
| Interrupt transfer | Yes (registers exist) | API only | **No** | No example uses interrupts |
| DMA transfer | No evidence | No | No | No DMA registers |
| Baud rate ÷4–÷2048 | Yes | Yes | **÷4, ÷8, ÷32** | 10 divider values; flash=÷4, radio=÷8, RFID=÷32 |
| Clock source known | **Unknown** | Assumed 100 MHz | Assumed | No documentation |
| Overrun detection | Yes | Yes | Not tested | OVERRUN bit in Status |

---

## 11. Device-Specific Configuration Summary (from vega-sdk examples)

| Device | SPI Mode | Baud Divider | Speed @ 100 MHz | CSAAT Pattern | Notes |
|--------|----------|--------------|-----------------|---------------|-------|
| AT25SF161 Flash | 3 | ÷4 (CFD_4) | 25 MHz | Multi-byte (cmd+addr+data) | Fast, CS held for entire transaction |
| W25Q32 Flash | 3 | ÷4 (CFD_4) | 25 MHz | Multi-byte | Same as AT25SF161 |
| nRF24L01 Radio | 0 | ÷8 (CFD_8) | 12.5 MHz | Per register (2-5 bytes) | Separate CE GPIO for mode control |
| RC522 RFID | 0 | ÷32 (CFD_32) | 3.125 MHz | Per register (2 bytes) | Slow, command+data per access |
| ILI9488 Display | 0/3 | ÷4-÷8 | 12.5-25 MHz | Multi-byte | TFT LCD, command+data |
| SSD1309 OLED | 0/3 | ÷4-÷8 | 12.5-25 MHz | Multi-byte | OLED display |
| MAX7219 Matrix | 0 | ÷8-÷16 | 6.25-12.5 MHz | Multi-byte | LED matrix driver |

**Key Observations from All Examples:**
- Uses **polling mode only** — no interrupts demonstrated in any example
- Manual chip select management via `CSAAT` bit — **universal pattern**
- Full-duplex simulated by alternating transmit dummy / receive data
- 8-bit transfers only (Dbits=0) — **no example uses other widths**
- No DMA usage shown
- **SPI Mode 0** for most peripherals (nRF24, RC522, displays), **Mode 3** for flash
- **Baud rate varies** by device tolerance (3-25 MHz)
- **CSAAT=HIGH before sequence, CSAAT=LOW after** — consistent across all drivers

---

## 11. Open Hardware Questions

1. **Peripheral Clock Source:** Is SPI clocked from 100 MHz system clock or a divided peripheral clock?
2. **SPI 2/3 vs QSPI:** Do SPI 2/3 share hardware with QSPI (same base region 0x10200100)?
3. **DBITS Field:** Does hardware actually support >8-bit transfers? What do values 1-15 mean?
4. **Slave Mode:** Is there undocumented slave capability? No register bits suggest it.
5. **TxData/RxData Bits 16-17:** What do slave select bits actually control?
6. **Maximum Baud Rate:** Is ÷4 (25 MHz @ 100 MHz) the true max, or can divider be bypassed?
7. **Timing Parameters:** Setup/hold times, SCK rise/fall times — not in BSP.
8. **Power/Reset State:** Control register reset values not documented.

---

## 12. Register Access Patterns (BSP → Hardware)

| Operation | BSP Function | Register Access Sequence |
|-----------|--------------|-------------------------|
| **Init** | `SPI_init()` | 1. Write Baudrate (÷16)<br>2. Write Control (Mode 3, MSB, Fixed, CS0, 8-bit, no intr, CSAAT=0) |
| **Config** | `SPI_config()` | Write Control register (16-bit) |
| **Set Baud** | `SPI_set_baud()` | Write Baudrate register (value << 4) |
| **Transmit** | `SPI_transmit()` | 1. Poll Status.BUSY=0<br>2. Poll Status.TX_HOLD_EMPTY=1<br>3. Write TxData |
| **Receive** | `SPI_receive()` | 1. Poll Status.RX_COMPLETE=1<br>2. Read RxData |
| **Wait Busy** | `SPI_wait_if_busy()` | Poll Status.BUSY=0 |
| **CSAAT Toggle** | `SPI_set_CSAAT_pin()` | Read-Modify-Write Control bit 8 |
| **CS Select** | `SPI_set_slave_select()` | Read-Modify-Write TxData bits 16-17 |
| **Enable Intr** | `SPI_set_intr()` | Read-Modify-Write Control bits 6-7 |
| **Check Overrun** | `SPI_check_overrun()` | Read Status bit 5 |

---

## 13. Files Referenced for This Analysis

| File | Key Sections |
|------|--------------|
| `spi.h` | 111-135 (register structs, macros), 33-36 (instances), 79-88 (baud), 90-99 (status bits) |
| `spi.c` | 49-69 (init), 137-144 (config), 168-173 (baud), 214-222 (overrun), 233-244 (intr), 253-261 (receive), 271-280 (transmit), 294-300 (wait), 311-319 (CSAAT), 360-381 (ISR reg), 390-410 (intr status) |
| `config.h` | 26 (SPI_BASE_ADDR) |
| `interrupt.h` | 68-70, 43-44 (IRQ mapping) |
| `at25sf161.c` (freertos/vega-sdk) | 43-64 (init), 73-102 (read), 112-147 (write) |
| `w25q32.c` (vega-sdk) | 43-66 (init), 75-103 (read), 113-151 (write) |
| `nrf24.c` (vega-sdk) | 18-37 (init), 145-161 (getData), 174-208 (send), 289-312 (transferSync), 315-348 (register R/W) |
| `rc522_rfid_demo.c` (vega-sdk) | 30-68 (init), 70-92 (register R/W), 99-229 (card communication) |
| `qspi.h` | 88, 159 (shared base address) |

---

## 14. Next Steps (Stage 3)

Proceed to implementation tracing:
1. Trace complete SPI transaction from API to register writes
2. Analyze initialization sequence in detail
3. Trace flash read/write command sequences
4. Identify any race conditions or timing dependencies
5. Document exact register write/read ordering

---
*Document generated as part of Stage 2 Hardware Analysis for Lattice SPI HAL development.*