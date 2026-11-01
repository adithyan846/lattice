# THEJAS32 port

This directory contains the **project configuration** for the THEJAS32
FreeRTOS port, not the port itself.

The actual `port.c`, `portASM.S`, startup code and trap handler come from:

```
third_party/freertos-thejas32/
```

or from the RISC-V port shipped with `FreeRTOS-Kernel` if it matches the core.

## Checklist

1. Confirm the core ISA (`rv32im`, `rv32imac`, ...) and ABI (`ilp32`).
2. Set `configCPU_CLOCK_HZ` to the measured core clock.
3. Set `configTOTAL_HEAP_SIZE` from the linker memory map, leaving room for
   stacks and the main stack.
4. Verify the trap vector, `ecall` yield path and timer (MTIME/MTIMECMP)
   addresses in the port.
5. Implement `frt_assert_failed`, `vApplicationStackOverflowHook` and
   `vApplicationMallocFailedHook` in `boards/thejas32/`.
6. Test context switching under load before enabling the flight tasks.

## Interrupt priorities

Document every interrupt priority in `boards/thejas32/pinmap.h` or a board
table. Any ISR that calls a FreeRTOS `FromISR` API must be at or below
`configMAX_SYSCALL_INTERRUPT_PRIORITY` as defined by the port.

## Floating point

`configUSE_FPU` is disabled by default. If the core has an FPU, enable it in
both the compiler flags and the port's context-save code. Mixing the two is a
classic source of silent corruption.
