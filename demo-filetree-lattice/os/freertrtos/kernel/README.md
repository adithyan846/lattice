# Kernel location

Place or submodule the upstream FreeRTOS kernel here:

```
third_party/FreeRTOS-Kernel/
├── tasks.c
├── queue.c
├── list.c
├── timers.c
├── event_groups.c
├── stream_buffer.c
├── include/
└── portable/
```

Recommended:

```sh
cd third_party
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
```

Pin a release tag (for example `V11.1.0`) and record it in the build log. Do not
edit the kernel in place; put project changes in `os/freertrtos/` or the board
port.
