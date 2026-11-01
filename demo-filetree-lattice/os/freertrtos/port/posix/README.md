# POSIX / SITL port

Use the upstream FreeRTOS POSIX port for host simulation:

```
third_party/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/
```

or the port bundled with your FreeRTOS release. The SITL build links:

- the real flight modules,
- a simulated board (`boards/sitl`),
- simulated sensors and a quadrotor plant (`tests/sitl/`),
- MAVLink over UDP.

This configuration is **not** used on the target. Keep the SITL stack sizes and
heap separate from the THEJAS32 configuration.
