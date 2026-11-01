#ifndef MODULES_FLIGHT_MODE_MANAGER_MODE_H
#define MODULES_FLIGHT_MODE_MANAGER_MODE_H

#include <stdbool.h>

#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FLIGHT_MODE_MANUAL = 0,
    FLIGHT_MODE_STABILIZED,
    FLIGHT_MODE_ALT_HOLD,
    FLIGHT_MODE_POS_HOLD,
    FLIGHT_MODE_MISSION,
    FLIGHT_MODE_RTL,
    FLIGHT_MODE_LAND,
    FLIGHT_MODE_COUNT,
} flight_mode_t;

typedef struct {
    vec3_t position_ned;
    vec3_t velocity_ned;
    float roll;
    float pitch;
    float yaw;
    float roll_rate;
    float pitch_rate;
    float yaw_rate;
    float thrust;
} mode_setpoint_t;

typedef struct {
    float roll_stick;
    float pitch_stick;
    float yaw_stick;
    float thrust_stick;

    vec3_t local_position_ned;
    vec3_t home_position_ned;
    /** Active navigation target from the navigator (mission, RTL, land). */
    vec3_t target_position_ned;
    float altitude_m;
    float heading_rad;
    float roll;
    float pitch;
    float roll_rate;
    float pitch_rate;
    float yaw_rate;

    bool attitude_valid;
    bool altitude_valid;
    bool position_valid;
    bool home_valid;
    bool target_valid;
    bool mission_active;

    mode_setpoint_t output;
} mode_context_t;

typedef struct {
    flight_mode_t id;
    const char *name;
    int (*enter)(mode_context_t *context);
    int (*update)(mode_context_t *context, float dt_s);
    void (*exit)(mode_context_t *context);
} flight_mode_ops_t;

const flight_mode_ops_t *flight_mode_get_ops(flight_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_FLIGHT_MODE_MANAGER_MODE_H */
