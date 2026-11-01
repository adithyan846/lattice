#ifndef MODULES_FLIGHT_MODE_MANAGER_MODE_UTILS_H
#define MODULES_FLIGHT_MODE_MANAGER_MODE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

float mode_clamp(float value, float min_value, float max_value);

/** Symmetric deadband with linear rescale outside the band. */
float mode_deadband(float value, float deadband);

/** Stick expo: preserves endpoints, softens the centre. expo in [0, 1]. */
float mode_expo(float value, float expo);

/** Map a normalised stick [-1, 1] to an angle in radians. */
float mode_stick_to_angle(float stick, float max_angle_rad);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_FLIGHT_MODE_MANAGER_MODE_UTILS_H */
