# SITL board shim

The SITL board replaces the target BSP with host-side implementations of the
HAL interfaces. It is used by `tests/sitl/` together with the FreeRTOS POSIX
port and a simulated quadrotor plant.

`hal_posix.c` is deliberately permissive: UART console writes go to `stdout`,
SPI/I2C transfers are no-ops, and SD/flash calls report "not present". The
simulation injects sensor data through the driver interfaces, not through the
HAL.

This board is **not** a target configuration. Do not ship it in firmware.
