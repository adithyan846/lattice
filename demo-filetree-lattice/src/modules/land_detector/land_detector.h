#ifndef MODULES_LAND_DETECTOR_LAND_DETECTOR_H
#define MODULES_LAND_DETECTOR_LAND_DETECTOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float altitude_m;
    float vertical_speed_mps;
    float thrust;
    float landed_time_s;
    float maybe_landed_time_s;
    float touchdown_altitude_m;
    float max_landed_speed_mps;
    float max_landed_thrust;
    bool landed;
    bool maybe_landed;
} land_detector_t;

void land_detector_init(land_detector_t *detector);

/**
 * Update the touchdown state machine. `altitude_m` is above ground, positive
 * up; `vertical_speed_mps` is positive up. Returns true when landed.
 */
bool land_detector_update(land_detector_t *detector, bool altitude_valid, float altitude_m,
                          float vertical_speed_mps, float thrust, float dt_s);

int land_detector_start(void);
int land_detector_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_LAND_DETECTOR_LAND_DETECTOR_H */
