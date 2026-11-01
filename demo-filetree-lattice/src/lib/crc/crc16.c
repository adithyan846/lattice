#include <lib/crc/crc16.h>

uint16_t crc16_accumulate(uint16_t crc, const uint8_t *data, size_t len)
{
    if (data == NULL) {
        return crc;
    }

    for (size_t i = 0; i < len; i++) {
        uint8_t tmp = (uint8_t)(data[i] ^ (crc & 0xFFu));
        tmp ^= (uint8_t)(tmp << 4);
        crc = (uint16_t)((crc >> 8) ^ ((uint16_t)tmp << 8) ^ ((uint16_t)tmp << 3) ^
                         ((uint16_t)tmp >> 4));
    }

    return crc;
}

uint16_t crc16_calculate(const uint8_t *data, size_t len)
{
    return crc16_accumulate(0xFFFFu, data, len);
}
