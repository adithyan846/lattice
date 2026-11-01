#ifndef MAIN_INIT_H
#define MAIN_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise board, kernel hooks, uORB and parameters. Returns 0 on success. */
int app_init(void);

/**
 * Start the flight modules. Each module owns its tasks. Returns the number of
 * modules that started successfully; zero means the scaffold is running without
 * flight functionality.
 */
int app_start_modules(void);

/** Stop modules in reverse order. */
void app_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_INIT_H */
