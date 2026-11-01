# Third-party dependencies

The firmware build intentionally does not vendor upstream code. Place pinned
checkouts here before enabling `FREERTRTOS_BUILD_FIRMWARE=ON`.

| Directory | Upstream | Used by |
|---|---|---|
| `FreeRTOS-Kernel/` | https://github.com/FreeRTOS/FreeRTOS-Kernel | scheduler, queues, timers |
| `freertos-thejas32/` | board/ISA port | RISC-V context switch and tick |
| `mavlink/` | https://github.com/mavlink/c_library_v2 | optional generated headers; the scaffold ships its own framing subset |
| `littlefs/` | https://github.com/littlefs-project/littlefs | optional SD-card filesystem |

Record the exact commit of each checkout in the release notes. Do not edit
upstream sources in place; carry patches as `.patch` files with an explanation.

The host tests and `tools/` scripts do not need any of these checkouts.
