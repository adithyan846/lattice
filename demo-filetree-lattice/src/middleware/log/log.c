#include <middleware/log/log.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <hal/uart.h>

static log_level_t current_level = LOG_LEVEL_INFO;

void log_init(log_level_t level)
{
    current_level = level;
}

void log_set_level(log_level_t level)
{
    current_level = level;
}

log_level_t log_get_level(void)
{
    return current_level;
}

void log_printf(log_level_t level, const char *format, ...)
{
    if (level > current_level || format == NULL) {
        return;
    }

    static const char prefixes[] = {'E', 'W', 'I', 'D'};
    char buffer[160];
    const int prefix_length = snprintf(buffer, sizeof(buffer), "[%c] ",
                                       prefixes[level > LOG_LEVEL_DEBUG ? LOG_LEVEL_DEBUG : level]);

    if (prefix_length < 0 || (size_t)prefix_length >= sizeof(buffer)) {
        return;
    }

    va_list arguments;
    va_start(arguments, format);
    const int body_length = vsnprintf(&buffer[prefix_length], sizeof(buffer) - (size_t)prefix_length,
                                      format, arguments);
    va_end(arguments);

    if (body_length < 0) {
        return;
    }

    size_t length = (size_t)prefix_length + (size_t)body_length;

    if (length >= sizeof(buffer) - 1) {
        length = sizeof(buffer) - 2;
    }

    buffer[length++] = '\n';
    (void)hal_uart_write(HAL_UART_CONSOLE, (const uint8_t *)buffer, length, 10);
}
