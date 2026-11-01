# Coding standards

## 1. Language and compiler

- C11 for flight code. C++ is allowed only in host tools, not in the firmware.
- `-Wall -Wextra`, no warnings in merged code.
- No compiler extensions unless the board port requires them; isolate them in
  `boards/` or `os/freertrtos/`.

## 2. Naming

| Item | Style | Example |
|---|---|---|
| Types | `snake_case_t` | `vehicle_attitude_s`, `pid_t` |
| Functions | `module_action()` | `commander_arm()`, `pid_update()` |
| Variables | `snake_case` | `position_setpoint` |
| Macros / constants | `UPPER_SNAKE` | `MISSION_MAX_ITEMS` |
| Message structs | `<topic>_s` | `vehicle_status_s` |
| Message metadata | `<topic>` | `vehicle_status` |
| Files | `snake_case.c/h` | `mc_att_control.c` |
| Params | `GROUP_NAME` | `MC_ROLLRATE_P` |

## 3. Real-time rules

1. No `malloc`/`free` in a periodic task.
2. No `printf` in a 1 kHz path; use a ring buffer or a debug build.
3. No blocking I/O in an ISR; post to a queue.
4. No unbounded loops; every loop has a documented bound.
5. No floating-point division by a value that can be zero without a guard.
6. Avoid `double` on a soft-float MCU; use `float` unless geodesy needs double
   (`lat/lon` in `lib/geo` is the exception).
7. Keep critical sections short; never call a driver inside one.
8. Check every return value that can fail.

## 4. Module boundaries

- A module includes its own header first, then middleware/lib headers, then
  system headers.
- A module must not include another module's private header.
- Cross-module communication is through uORB topics or a documented library
  API.
- Drivers do not include flight modules.
- No global mutable state outside the module's `.c` file (use a static struct).

## 5. Error handling

- Return `0` on success, negative errno-style values on failure.
- Log a human-readable reason once, not every loop iteration.
- Failsafe decisions belong to `commander`, not to a driver.
- A missing optional device (e.g. SD card) must not prevent boot unless the
  board config says it is required.

## 6. Parameters

- Every tunable is a parameter; no magic numbers in control laws.
- Parameters have a safe default and a documented range.
- Read parameters at init or on `parameter_update`, not every loop.
- Names are ≤ 16 characters, uppercase, `GROUP_` prefix.

## 7. Logging and diagnostics

- `FRT_LOG_ERROR/WARN/INFO/DEBUG` from `middleware/log/log.h`.
- Use `mavlink_send_statustext()` for operator-visible events.
- Never log at high rate from a control task; use the logger's topic list.
- Include the module name in every message.

## 8. Comments and docs

- File header: purpose, owner, and any safety-relevant notes.
- Public functions: what it does, units, range, blocking behaviour.
- Units in names or comments (`_m`, `_ms`, `_rad`, `_deg`).
- `TODO(owner):` for unimplemented work, with a linked issue if possible.
- Keep comments truthful; do not claim a function is implemented when it is a
  stub.

## 9. Formatting

Run `make format` before committing. The style is defined in `.clang-format`
(4 spaces, 100 columns, braces attached). CI runs a format check.
