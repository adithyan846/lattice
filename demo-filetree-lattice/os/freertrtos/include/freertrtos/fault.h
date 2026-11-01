#ifndef FREERTRTOS_FAULT_H
#define FREERTRTOS_FAULT_H

/**
 * Fault hooks implemented by the board support package. They must be visible:
 * toggle an LED, print to the console and reset if the fault is unrecoverable.
 */

#ifdef __cplusplus
extern "C" {
#endif

void frt_assert_failed(const char *file, int line);
void frt_stack_overflow_hook(void *task_handle, const char *task_name);
void frt_malloc_failed_hook(void);
void frt_fault_reboot(void);

#ifdef __cplusplus
}
#endif

#endif /* FREERTRTOS_FAULT_H */
