# freertrtos

`freertrtos` is the operating-system adaptation layer for this autopilot. It is
not a fork of FreeRTOS: it wraps the upstream FreeRTOS kernel with:

- a board-specific `FreeRTOSConfig.h`,
- typed task/queue/mutex/time wrappers (`frt_*`),
- boot-time kernel-object creation,
- fault hooks (assert, stack overflow, malloc failure),
- a single place to review real-time policy.

The spelling **freertrtos** is intentional for this project. The upstream
kernel and RISC-V port keep their own names under `third_party/`.

## Layout

```
os/freertrtos/
├── include/freertrtos/     Public wrappers used by flight code
├── src/                    Wrapper implementations
├── port/thejas32/          FreeRTOSConfig.h for the reference board
├── port/posix/             FreeRTOSConfig.h for SITL
└── kernel/                 Placeholder / notes for the upstream kernel
```

## Getting the kernel

The firmware build expects:

```
third_party/FreeRTOS-Kernel/       # upstream kernel
third_party/freertos-thejas32/     # THEJAS32 port
```

Clone them as described in `docs/build.md`. The build fails with a clear error
if either is missing. Host unit tests do not need them.

## Design rules

1. Flight code includes `<freertrtos/...>`, never `FreeRTOS.h` directly. This
   keeps the OS boundary visible and makes a future port easier.
2. All periodic tasks are created statically at boot by
   `src/main/task_registry.c`.
3. No dynamic allocation after init in the flight path.
4. The tick rate is 1 kHz. If a module needs sub-millisecond timing, use a
   hardware timer or the IMU data-ready interrupt.
5. `configASSERT` must be visible: log the file/line and reset the board.

## Fault hooks

Implement these in the board support package:

```c
void frt_assert_failed(const char *file, int line);
void vApplicationStackOverflowHook(TaskHandle_t task, char *name);
void vApplicationMallocFailedHook(void);
```

The scaffold declares them in `include/freertrtos/fault.h`; the board provides
the LED/UART/reset behaviour.
