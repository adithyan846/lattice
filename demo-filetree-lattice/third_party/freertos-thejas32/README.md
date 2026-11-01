# THEJAS32 FreeRTOS port

Place the board/ISA port here. `cmake/freertrtos.cmake` looks for either:

- `port.c` at the top level, or
- `portable/GCC/RISC-V/port.c` and `portASM.S`.

The port must provide the trap vector, tick interrupt, context switch and FPU
handling required by the board. Verify the interrupt priority mapping against
`os/freertrtos/include/FreeRTOSConfig.h` before booting.
