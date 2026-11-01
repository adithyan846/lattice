#include <drivers/pwm_out/dshot.h>

static uint16_t dshot_crc(uint16_t packet)
{
    return (uint16_t)((packet ^ (packet >> 4) ^ (packet >> 8)) & 0x0Fu);
}

uint16_t dshot_encode(float throttle_normalized, bool request_telemetry)
{
    if (throttle_normalized < 0.0f) {
        throttle_normalized = 0.0f;
    } else if (throttle_normalized > 1.0f) {
        throttle_normalized = 1.0f;
    }

    uint16_t throttle = (uint16_t)(throttle_normalized * (float)DSHOT_MAX_THROTTLE + 0.5f);

    if (throttle < DSHOT_MIN_THROTTLE) {
        throttle = DSHOT_MIN_THROTTLE;
    }

    if (throttle > DSHOT_MAX_THROTTLE) {
        throttle = DSHOT_MAX_THROTTLE;
    }

    uint16_t packet = (uint16_t)((throttle << 1) | (request_telemetry ? 1u : 0u));
    return (uint16_t)((packet << 4) | dshot_crc(packet));
}
