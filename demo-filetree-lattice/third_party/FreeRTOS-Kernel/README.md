# FreeRTOS-Kernel

Place a pinned checkout of https://github.com/FreeRTOS/FreeRTOS-Kernel here.

The firmware build expects at least:

- `tasks.c`
- `queue.c`
- `list.c`
- `timers.c`
- `event_groups.c`
- `stream_buffer.c`
- `portable/MemMang/heap_4.c`

Do not copy files into this directory by hand; clone or submodule the upstream
repository and record the commit hash.
