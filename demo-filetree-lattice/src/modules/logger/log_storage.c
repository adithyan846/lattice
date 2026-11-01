#include <modules/logger/log_storage.h>

#include <stdio.h>
#include <string.h>

void log_storage_init(log_storage_t *storage, uint32_t max_file_bytes)
{
    if (storage == NULL) {
        return;
    }

    storage->max_file_bytes = max_file_bytes > 0 ? max_file_bytes : (64U * 1024U * 1024U);
    storage->file_bytes = 0;
    storage->file_index = 0;
    storage->truncated = false;
}

int log_storage_path(char *buffer, size_t capacity, const char *root, const char *date,
                     const char *time, uint32_t index)
{
    if (buffer == NULL || capacity == 0 || root == NULL || date == NULL || time == NULL) {
        return -1;
    }

    int written;

    if (index == 0) {
        written = snprintf(buffer, capacity, "%s/%s/%s_freertrtos.ftlog", root, date, time);

    } else {
        written = snprintf(buffer, capacity, "%s/%s/%s_%u_freertrtos.ftlog", root, date, time,
                           index);
    }

    if (written < 0 || (size_t)written >= capacity) {
        return -1;
    }

    return 0;
}

bool log_storage_should_rotate(const log_storage_t *storage, size_t incoming_bytes)
{
    if (storage == NULL) {
        return false;
    }

    return (uint64_t)storage->file_bytes + incoming_bytes > storage->max_file_bytes;
}

void log_storage_note_written(log_storage_t *storage, size_t bytes)
{
    if (storage != NULL) {
        storage->file_bytes += (uint32_t)bytes;
    }
}

void log_storage_begin_file(log_storage_t *storage)
{
    if (storage != NULL) {
        storage->file_bytes = 0;
        storage->file_index++;
    }
}

void log_storage_mark_truncated(log_storage_t *storage)
{
    if (storage != NULL) {
        storage->truncated = true;
    }
}
