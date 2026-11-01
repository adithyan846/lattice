#ifndef MIDDLEWARE_LOG_LOG_H
#define MIDDLEWARE_LOG_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} log_level_t;

void log_init(log_level_t level);
void log_set_level(log_level_t level);
log_level_t log_get_level(void);

/** printf-style log to the console UART. Not for 1 kHz paths. */
void log_printf(log_level_t level, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

#define FRT_LOG_ERROR(...) log_printf(LOG_LEVEL_ERROR, __VA_ARGS__)
#define FRT_LOG_WARN(...)  log_printf(LOG_LEVEL_WARN, __VA_ARGS__)
#define FRT_LOG_INFO(...)  log_printf(LOG_LEVEL_INFO, __VA_ARGS__)
#define FRT_LOG_DEBUG(...) log_printf(LOG_LEVEL_DEBUG, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_LOG_LOG_H */
