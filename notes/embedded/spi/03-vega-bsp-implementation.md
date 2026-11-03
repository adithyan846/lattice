# VEGA BSP SPI Implementation Tracing

**Project:** Lattice Flight Controller (THEJAS32 / RV32IM)
**Date:** 2026-11-03
**Stage:** 3 — BSP Implementation Trace
**Status:** Complete

---

## 1. Initialization Sequence Trace

### 1.1 `SPI_init(UC spi_number)` — `spi.c:49-69`

```c
void SPI_init(UC spi_number) {
    UC rx = 0;                    // Unused variable
    UI reg_data = 0;              // Unused variable

    SPIreg(spi_number).Baudrate = (SPI_BAUD_CFD_16 << 4);  // 0x20 = ÷16

    gSPItransfer.Bits.Dbits = 8;          // 8-bit transfers
    gSPItransfer.Bits.CSAAT = LOW;        // CS deasserts after each byte
    gSPItransfer.Bits.SPIrxTxIntr = RX_TX_INTR_DIS;  // No interrupts
    gSPItransfer.Bits.Mode = 3;           // SPI Mode 3 (CPOL=1, CPHA=1)
    gSPItransfer.Bits.DataOrder = MSB;    // MSB first
    gSPItransfer.Bits.PeriphType = 0;     // Fixed peripheral (CS0)
    gSPItransfer.Bits.PeriphChipSelect = 0; // CS0

    SPIreg(spi_number).Control.hword = gSPItransfer.Value;  // Write Control
    __asm__ __volatile__ ("fence");       // Memory barrier
    return;
}
```

**Register Writes:**
| Order | Register | Value | Meaning |
|-------|----------|-------|---------|
| 1 | Baudrate | `0x20` (2 << 4) | ÷16 divider → 6.25 MHz @ 100 MHz |
| 2 | Control | `0x2B80` | Mode 3, MSB, Fixed, CS0, 8-bit, no intr, CSAAT=0 |

**Control Register Breakdown (0x2B80):**
```
Bits 15:13 = 000 (Reserved)
Bits 12:9  = 0000 (DBITS=0 → 8-bit)
Bit 8      = 0    (CSAAT=0)
Bit 7      = 0    (SPTIE=0)
Bit 6      = 0    (SPRIE=0)
Bit 5      = 1    (CPOL=1)
Bit 4      = 1    (CPHA=1)
Bit 3      = 0    (MSB first)
Bit 2      = 0    (PS=0 Fixed)
Bits 1:0   = 00   (PCS=00 → CS0)
= 0b0010_1011_1000_0000 = 0x2B80
```

### 1.2 `SPI_config(UC spi_number, SPIcntrlRegType spi_control_reg)` — `spi.c:137-144`

```c
void SPI_config(UC spi_number, SPIcntrlRegType spi_control_reg) {
    SPIreg(spi_number).Control.hword = spi_control_reg.Value;
    __asm__ __volatile__ ("fence");
    return;
}
```

**Single register write** — allows full customization after init.

### 1.3 `SPI_set_baud(UC spi_number, UC bBaudCFD)` — `spi.c:168-173`

```c
void SPI_set_baud(UC spi_number, UC bBaudCFD) {
    SPIreg(spi_number).Baudrate = (bBaudCFD << 4);
    __asm__ __volatile__ ("fence");
    return;
}
```

**Baudrate register:** Value shifted left by 4 bits.

---

## 2. Chip Select Management Trace

### 2.1 `SPI_set_CSAAT_pin(UC spi_number, UC status)` — `spi.c:311-319`

```c
void SPI_set_CSAAT_pin(UC spi_number, UC status) {
    if (status == 1)
        SPIreg(spi_number).Control.hword |= (1 << 8);   // Set bit 8
    else
        SPIreg(spi_number).Control.hword &= ~(1 << 8);  // Clear bit 8
    __asm__ __volatile__ ("fence");
    return;
}
```

**Read-Modify-Write** on Control register bit 8 (CSAAT).

**Usage Pattern (from examples):**
```c
SPI_wait_if_busy(spi);           // Ensure idle
SPI_set_CSAAT_pin(spi, HIGH);    // Assert CS (hold low)
// ... multiple SPI_transmit() calls ...
SPI_set_CSAAT_pin(spi, LOW);     // Deassert CS
```

### 2.2 `SPI_set_slave_select(UC spi_number, UI slave_sel_val)` — `spi.c:198-205`

```c
void SPI_set_slave_select(UC spi_number, UI slave_sel_val) {
    UI write_data = 0, read_data = 0;
    read_data = SPIreg(spi_number).TxData;
    write_data = read_data | (slave_sel_val << 16);
    SPIreg(spi_number).TxData = write_data;
    return;
}
```

**Writes to TxData bits 16-17** — purpose unclear, possibly for variable peripheral mode.

### 2.3 `SPI_get_slave_select(UC spi_number)` — `spi.c:182-188`

```c
UI SPI_get_slave_select(UC spi_number) {
    UI read_data = 0;
    read_data = SPIreg(spi_number).RxData;
    read_data = ((read_data & 0x30000) >> 16);
    return read_data;
}
```

**Reads from RxData bits 16-17** — mirrors TxData slave select bits.

---

## 3. Data Transfer Trace

### 3.1 `SPI_wait_if_busy(UC spi_number)` — `spi.c:294-300`

```c
void SPI_wait_if_busy(UC spi_number) {
    while (SPIreg(spi_number).Status & SPI_BUSY_BIT)  // Bit 4
        ;
    return;
}
```

**Polls Status bit 4 (BUSY)** until controller idle.

### 3.2 `SPI_transmit(UC spi_number, US bData)` — `spi.c:271-280`

```c
void SPI_transmit(UC spi_number, US bData) {
    SPI_wait_if_busy(spi_number);                          // 1. Wait BUSY=0
    while (!(SPIreg(spi_number).Status & SPI_TX_HOLD_EMPTY_BIT))  // 2. Wait TX_HOLD_EMPTY=1
        ;
    SPIreg(spi_number).TxData = bData;                     // 3. Write data
    __asm__ __volatile__ ("fence");
    return;
}
```

**Three-Step Sequence:**
1. **Wait for previous transfer complete** (BUSY=0)
2. **Wait for TX hold register empty** (TX_HOLD_EMPTY=1)
3. **Write TxData** — hardware starts shift immediately

**Note:** Only low 8 bits of `bData` used (8-bit transfers).

### 3.3 `SPI_receive(UC spi_number)` — `spi.c:253-261`

```c
US SPI_receive(UC spi_number) {
    US bRxData;
    UC status = 0;
    while (!((SPIreg(spi_number).Status) & SPI_RX_COMPLETE_BIT))  // Wait RX_COMPLETE=1
        ;
    bRxData = SPIreg(spi_number).RxData;   // Read data
    return bRxData;
}
```

**Single Step:** Poll Status bit 6 (RX_COMPLETE), then read RxData.

### 3.4 `SPI_write_tx_reg(UC spi_number, US bData)` — `spi.c:342-350`

```c
void SPI_write_tx_reg(UC spi_number, US bData) {
    SPI_wait_if_busy(spi_number);
    while (!(SPIreg(spi_number).Status & SPI_TX_HOLD_EMPTY_BIT))
        ;
    SPIreg(spi_number).TxData = bData;
    __asm__ __volatile__ ("fence");
    return;
}
```

**Identical to `SPI_transmit`** — different name, same implementation.

### 3.5 `SPI_read_rx_reg(UC spi_number)` — `spi.c:328-332`

```c
US SPI_read_rx_reg(UC spi_number) {
    US bRxData;
    bRxData = SPIreg(spi_number).RxData;
    return bRxData;
}
```

**No wait** — direct register read. Caller must ensure data ready.

---

## 4. Complete Transaction Trace: Flash Read (AT25SF161)

From `at25sf161.c:73-102` (`at25sf161ReadEeprom`):

```c
void at25sf161ReadEeprom(char *readBuf, int length, unsigned int eepromAddress) {
    char cmdBuf[4];
    cmdBuf[0] = READ_EEPROM;                    // 0x03
    cmdBuf[1] = (eepromAddress >> 16) & 0xFF;   // Addr[23:16]
    cmdBuf[2] = (eepromAddress >> 8) & 0xFF;    // Addr[15:8]
    cmdBuf[3] = (eepromAddress) & 0xFF;         // Addr[7:0]

    SPI_wait_if_busy(currentSpi);               // [1] Wait idle
    SPI_set_CSAAT_pin(currentSpi, HIGH);        // [2] Set CSAAT=1 (hold CS)

    // [3] Transmit 4-byte command + address (CS held low)
    SPI_transmit(currentSpi, cmdBuf[0]);        // TX: 0x03, RX: dummy
    SPI_transmit(currentSpi, cmdBuf[1]);        // TX: addr[23:16], RX: dummy
    SPI_transmit(currentSpi, cmdBuf[2]);        // TX: addr[15:8], RX: dummy
    SPI_transmit(currentSpi, cmdBuf[3]);        // TX: addr[7:0], RX: dummy

    // [4] Read data bytes (full-duplex: TX dummy, RX data)
    while (length) {
        SPI_transmit(currentSpi, DUMMY_DATA);   // TX: 0x00
        *readBuf = SPI_receive(currentSpi);     // RX: flash data
        readBuf++;
        length--;
    }

    SPI_set_CSAAT_pin(currentSpi, LOW);         // [5] Set CSAAT=0 (release CS)
}
```

### Register-Level Timeline

| Step | Operation | Register Access | Hardware State |
|------|-----------|-----------------|----------------|
| [1] | `SPI_wait_if_busy` | Poll Status.BUSY=0 | Controller idle |
| [2] | `SPI_set_CSAAT_pin(HIGH)` | Control.CSAAT=1 | CS will stay low |
| [3a] | `SPI_transmit(0x03)` | Wait BUSY=0, TX_EMPTY=1 → Write TxData=0x03 | Shift 8 bits, CS low |
| [3b] | `SPI_transmit(addr[23:16])` | Wait BUSY=0, TX_EMPTY=1 → Write TxData=addr | Shift 8 bits, CS low |
| [3c] | `SPI_transmit(addr[15:8])` | Wait BUSY=0, TX_EMPTY=1 → Write TxData=addr | Shift 8 bits, CS low |
| [3d] | `SPI_transmit(addr[7:0])` | Wait BUSY=0, TX_EMPTY=1 → Write TxData=addr | Shift 8 bits, CS low |
| [4a] | `SPI_transmit(0x00)` | Wait BUSY=0, TX_EMPTY=1 → Write TxData=0x00 | Shift 8 bits, CS low |
| [4b] | `SPI_receive()` | Poll RX_COMPLETE=1 → Read RxData | Data captured from MISO |
| [5] | `SPI_set_CSAAT_pin(LOW)` | Control.CSAAT=0 | CS deasserts (high) |

**Electrical CS Waveform:**
```
CS:  _______/¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\_______
       ↑                                    ↑
    CSAAT=1                              CSAAT=0
   (after wait)                          (after last byte)
```

---

## 5. Interrupt Handling Trace

### 5.1 `SPI_set_intr(UC spi_number, UC tx_intr, UC rx_intr)` — `spi.c:233-244`

```c
void SPI_set_intr(UC spi_number, UC tx_intr, UC rx_intr) {
    UC intr_val = 0;
    US cword = 0;
    cword = SPIreg(spi_number).Control.hword;        // Read current
    intr_val = ((tx_intr << 7) | (rx_intr << 6));    // Bit 7=TX, Bit 6=RX
    cword &= 0xFF3F;                                 // Clear bits 7:6
    cword |= intr_val;                               // Set new values
    SPIreg(spi_number).Control.hword = cword;        // Write back
    __asm__ __volatile__ ("fence");
    return;
}
```

**Read-Modify-Write** on Control bits 7 (SPTIE) and 6 (SPRIE).

### 5.2 `spi_register_isr(UC spi_number, void (*spi_isr)())` — `spi.c:360-381`

```c
void spi_register_isr(UC spi_number, void (*spi_isr)()) {
    UC irq_no;
    #if __riscv_xlen == 64
        // SPI 0=6, 1=7, 2=52, 3=53
    #else
        if (spi_number == 0) irq_no = 3;
        else if (spi_number == 1) irq_no = 4;
        else if (spi_number == 2) irq_no = 23;
    #endif
    interrupt_enable(irq_no);           // Enable in PLIC
    irq_register_handler(irq_no, spi_isr); // Register handler
}
```

**Maps SPI instance → IRQ number → enables interrupt.**

### 5.3 `spi_intr_status(UC spi_number)` — `spi.c:390-410`

```c
UC spi_intr_status(UC spi_number) {
    UC status = 0;
    if (spi_number == 0) status = SPIreg(0).Status;
    if (spi_number == 1) status = SPIreg(1).Status;
    if (spi_number == 2) status = SPIreg(2).Status;
    if (spi_number == 3) status = SPIreg(3).Status;

    if (status & (1 << 2)) return SPI_RX_INTR;  // Bit 2
    else if (status & (1 << 3)) return SPI_TX_INTR; // Bit 3
}
```

**Handler must call this to determine interrupt source.**

---

## 6. Error Handling Trace

### 6.1 `SPI_check_overrun(UC spi_number)` — `spi.c:214-222`

```c
UC SPI_check_overrun(UC spi_number) {
    UC status = 0;
    status = SPIreg(spi_number).Status;
    if (status & (1 << 5))  // Bit 5 = OVERRUN
        return 1;
    else
        return 0;
}
```

**Polls Status bit 5** — returns 1 if overrun occurred.

**Overrun Condition:** New data received in shift register before previous RxData read.

---

## 7. Example Usage Patterns from vega-sdk

### 7.1 SPI Flash (AT25SF161, W25Q32) — Mode 3, Fast
```c
SPI_config: Mode=3, MSB, Fixed, CS0, 8-bit, no intr, CSAAT=0
SPI_set_baud: SPI_BAUD_CFD_4 (÷4 = 25 MHz)
Transaction: CSAAT=HIGH → 4-byte CMD+ADDR → N×(TX dummy + RX data) → CSAAT=LOW
```

### 7.2 nRF24L01 Radio — Mode 0, Medium, GPIO CE
```c
SPI_config: Mode=0, MSB, Fixed, CS0, 8-bit, no intr, CSAAT=0
SPI_set_baud: SPI_BAUD_CFD_8 (÷8 = 12.5 MHz)
Helpers: nrf24_transferSync() (full-duplex), nrf24_transmitSync() (TX only)
CE pin: Separate GPIO control (not SPI CS)
```

### 7.3 RC522 RFID — Mode 0, Slow
```c
SPI_config: Mode=0, MSB, Fixed, CS0, 8-bit, no intr, CSAAT=0
SPI_set_baud: SPI_BAUD_CFD_32 (÷32 = 3.125 MHz)
Register R/W: CSAAT=HIGH → TX cmd_byte → TX/RX data → CSAAT=LOW (per register)
```

### 7.4 Display Drivers (ILI9488, SSD1309, MAX7219)
- Similar to flash: Mode 0 or 3, CSAAT for multi-byte
- Often use `SPI_transmit` for commands, `SPI_write_tx_reg` for data streaming

---

## 8. Critical Implementation Observations

### 8.1 Global State Variable
```c
SPIcntrlRegType gSPItransfer;  // spi.c:39 — global, not per-instance!
```
**Issue:** `SPI_init()` uses global `gSPItransfer` — **not thread-safe** for multi-instance use.

### 8.2 No Per-Instance Configuration Storage
- `SPI_init()` writes defaults to hardware
- `SPI_config()` overwrites entirely
- No API to read back current configuration
- Caller must track configuration state

### 8.3 Busy-Wait Only (No Timeouts)
```c
while (SPIreg(spi_number).Status & SPI_BUSY_BIT) ;  // Infinite wait
```
**Risk:** Hardware hang → infinite loop. No timeout parameter.

### 8.4 No Buffer/Array Transfer APIs
- Only single-byte `SPI_transmit`/`SPI_receive`
- Callers must loop for multi-byte
- Each byte: 2-3 status polls + register write/read

### 8.5 Memory Barriers Used
```c
__asm__ __volatile__ ("fence");  // After every register write
```
Ensures write ordering on weakly-ordered RISC-V.

### 8.6 3 SPI Instances Physically Available (User Confirmed)
- SPI 0, 1, 2 have IRQs on RV32 (3, 4, 23)
- SPI 3 IRQ only on RV64
- Examples use SPI 0 predominantly

---

## 9. Function Call Dependency Graph

```
SPI_init(spi)
  ├─ SPIreg(spi).Baudrate = (2 << 4)
  └─ SPIreg(spi).Control = gSPItransfer.Value

SPI_config(spi, ctrl)
  └─ SPIreg(spi).Control = ctrl.Value

SPI_set_baud(spi, div)
  └─ SPIreg(spi).Baudrate = (div << 4)

SPI_transmit(spi, data)
  ├─ SPI_wait_if_busy(spi)
  │   └─ Poll SPIreg(spi).Status & BUSY_BIT
  ├─ Poll SPIreg(spi).Status & TX_HOLD_EMPTY_BIT
  └─ SPIreg(spi).TxData = data

SPI_receive(spi)
  ├─ Poll SPIreg(spi).Status & RX_COMPLETE_BIT
  └─ return SPIreg(spi).RxData

SPI_set_CSAAT_pin(spi, val)
  ├─ Read SPIreg(spi).Control
  ├─ Modify bit 8
  └─ Write SPIreg(spi).Control

SPI_set_intr(spi, tx, rx)
  ├─ Read SPIreg(spi).Control
  ├─ Modify bits 7,6
  └─ Write SPIreg(spi).Control

spi_register_isr(spi, handler)
  ├─ Map spi → irq_no
  ├─ interrupt_enable(irq_no)
  └─ irq_register_handler(irq_no, handler)

spi_intr_status(spi)
  ├─ Read SPIreg(spi).Status
  ├─ Check bit 2 → SPI_RX_INTR
  └─ Check bit 3 → SPI_TX_INTR
```

---

## 10. Files Referenced

| File | Functions Traced |
|------|------------------|
| `spi.c` | All 14 public APIs |
| `at25sf161.c` (freertos & vega-sdk) | Flash read/write/erase |
| `w25q32.c` (vega-sdk) | Flash read/write |
| `nrf24.c` (vega-sdk) | Radio config, TX/RX, full-duplex helpers |
| `rc522_rfid_demo.c` (vega-sdk) | Register R/W, card communication |

---

## 11. Next Steps (Stage 4)

Proceed to HAL design:
1. Define portable `hal_spi.h` interface
2. Map HAL operations → BSP operations
3. Classify features: essential / future / platform-specific
4. Design platform translator (`thejas32_spi.c`)
5. Handle CS management, timeouts, error reporting

---
*Document generated as part of Stage 3 Implementation Tracing for Lattice SPI HAL development.*