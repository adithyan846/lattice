#ifndef LIB_CRC_CRC32_H
#define LIB_CRC_CRC32_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Standard IEEE 802.3 CRC-32 (reflected, poly 0xEDB88320). */
uint32_t crc32_accumulate(uint32_t crc, const uint8_t *data, size_t len);

/** One-shot helper. */
uint32_t crc32_calculate(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* LIB_CRC_CRC32_H */
