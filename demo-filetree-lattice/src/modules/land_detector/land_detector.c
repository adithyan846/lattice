#include <modules/land_detector/land_detector.h>

#include <stddef.h>

#include <middleware/log/log.h>

#define LANDED_CONFIRM_TIME_S 1.0f
#define MAYBE_LANDED_TIME_S 0.3f
#define LANDED_ALTITUDE_M 0.20f
#define LANDED_SPEED_MPS 0.30f
#define LANDED_THRUST_MAX 0.35f

void land_detector_init(land_detector_t *detector)
{
    if (detector == NULL) {
        return;
    }

    *detector = (land_detector_t){0};
    detector->touchdown_altitude_m = LANDED_ALTITUDE_M;
    detector->max_landed_speed_mps = LANDED_SPEED_MPS;
    detector->max_landed_thrust = LANDED_THRUST_MAX;
}

bool land_detector_update(land_detector_t *detector, bool altitude_valid, float altitude_m,
                          float vertical_speed_mps, float thrust, float dt_s)
{
    if (detector == NULL) {
        return false;
    }

    detector->altitude_m = altitude_m;
    detector->vertical_speed_mps = vertical_speed_mps;
    detector->thrust = thrust;

    const bool near_ground = altitude_valid && altitude_m <= detector->touchdown_altitude_m;
    const bool slow = vertical_speed_mps < detector->max_landed_speed_mps &&
                      vertical_speed_mps > -detector->max_landed_speed_mps;
    const bool low_thrust = thrust <= detector->max_landed_thrust;

    if (near_ground && slow && low_thrust) {
        detector->maybe_landed_time_s += dt_s > 0.0f ? dt_s : 0.0f;

        if (detector->maybe_landed_time_s >= MAYBE_LANDED_TIME_S) {
            detector->maybe_landed = true;
        }

        if (detector->maybe_landed_time_s >= LANDED_CONFIRM_TIME_S) {
            detector->landed = true;
        }

    } else {
        detector->maybe_landed_time_s = 0.0f;
        detector->maybe_landed = false;
        detector->landed = false;
    }

    return detector->landed;
}

int land_detector_start(void)
{
    /* TODO: subscribe to vehicle_local_position and actuator outputs, run the
     * detector at 50 Hz and publish vehicle_land_detected. */
    FRT_LOG_WARN("land_detector: scaffold only, detector task not implemented");
    return -1;
}

int land_detector_stop(void)
{
    return 0;
}
