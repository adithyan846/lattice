#include <modules/logger/log_writer.h>

#include <stddef.h>
#include <string.h>

static int flush_buffer(log_writer_t *writer)
{
    if (writer->used == 0) {
        return 0;
    }

    if (writer->backend.write == NULL) {
        writer->write_errors++;
        writer->used = 0;
        return -1;
    }

    const int result = writer->backend.write(writer->backend.context, writer->buffer, writer->used);

    if (result != 0) {
        writer->write_errors++;
        writer->used = 0;
        return -1;
    }

    writer->bytes_written += (uint32_t)writer->used;
    writer->used = 0;
    return 0;
}

void log_writer_init(log_writer_t *writer, const log_backend_t *backend)
{
    if (writer == NULL) {
        return;
    }

    memset(writer, 0, sizeof(*writer));

    if (backend != NULL) {
        writer->backend = *backend;
    }
}

int log_writer_write(log_writer_t *writer, const uint8_t *data, size_t length)
{
    if (writer == NULL || (data == NULL && length > 0)) {
        return -1;
    }

    while (length > 0) {
        const size_t free_space = sizeof(writer->buffer) - writer->used;
        const size_t chunk = length < free_space ? length : free_space;

        memcpy(&writer->buffer[writer->used], data, chunk);
        writer->used += chunk;
        data += chunk;
        length -= chunk;

        if (writer->used == sizeof(writer->buffer)) {
            if (flush_buffer(writer) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

int log_writer_flush(log_writer_t *writer)
{
    if (writer == NULL) {
        return -1;
    }

    const int result = flush_buffer(writer);

    if (writer->backend.flush != NULL && writer->backend.flush(writer->backend.context) != 0) {
        return -1;
    }

    return result;
}

void log_writer_reset(log_writer_t *writer)
{
    if (writer != NULL) {
        writer->used = 0;
    }
}
