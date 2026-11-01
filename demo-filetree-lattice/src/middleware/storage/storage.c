#include <middleware/storage/storage.h>

/* TODO: wrap littlefs/FatFs over hal_sd_* and expose the tiny API above.
 * Keep the logger's write path isolated from the flight tasks. */

int storage_mount(void)
{
    return -1;
}

bool storage_ready(void)
{
    return false;
}

int storage_read_file(const char *path, void *buffer, size_t max_length, size_t *out_length)
{
    (void)path;
    (void)buffer;
    (void)max_length;
    (void)out_length;
    return -1;
}

int storage_write_file(const char *path, const void *data, size_t length)
{
    (void)path;
    (void)data;
    (void)length;
    return -1;
}

int storage_append_file(const char *path, const void *data, size_t length)
{
    (void)path;
    (void)data;
    (void)length;
    return -1;
}
