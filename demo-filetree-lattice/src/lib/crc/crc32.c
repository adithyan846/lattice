#include <lib/crc/crc32.h>

uint32_t crc32_accumulate(uint32_t crc, const uint8_t *data, size_t len)
{
    if (data == NULL) {
        return crc;
    }

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++) {
            const uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return crc;
}

uint32_t crc32_calculate(const uint8_t *data, size_t len)
{
    return crc32_accumulate(0xFFFFFFFFu, data, len) ^ 0xFFFFFFFFu;
}
