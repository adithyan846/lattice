#include <modules/flight_mode_manager/mode_utils.h>

#include <math.h>

float mode_clamp(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

float mode_deadband(float value, float deadband)
{
    if (deadband <= 0.0f) {
        return value;
    }

    if (value > deadband) {
        return (value - deadband) / (1.0f - deadband);
    }

    if (value < -deadband) {
        return (value + deadband) / (1.0f - deadband);
    }

    return 0.0f;
}

float mode_expo(float value, float expo)
{
    const float clamped = mode_clamp(value, -1.0f, 1.0f);
    const float exponent = mode_clamp(expo, 0.0f, 1.0f);
    return (1.0f - exponent) * clamped + exponent * clamped * clamped * clamped;
}

float mode_stick_to_angle(float stick, float max_angle_rad)
{
    return mode_clamp(stick, -1.0f, 1.0f) * max_angle_rad;
}
