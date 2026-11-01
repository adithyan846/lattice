#ifndef LIB_CRC_CRC16_H
#define LIB_CRC_CRC16_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** MAVLink X.25 CRC-16. Start with 0xFFFF for a new frame. */
uint16_t crc16_accumulate(uint16_t crc, const uint8_t *data, size_t len);

/** One-shot helper. */
uint16_t crc16_calculate(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* LIB_CRC_CRC16_H */
