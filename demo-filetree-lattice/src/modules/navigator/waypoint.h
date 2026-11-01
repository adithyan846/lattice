#ifndef MODULES_NAVIGATOR_WAYPOINT_H
#define MODULES_NAVIGATOR_WAYPOINT_H

#include <stdbool.h>

#include <lib/geo/geo.h>
#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default vertical acceptance when an item does not specify one. */
#define WAYPOINT_DEFAULT_ALT_ACCEPT_M 3.0f

typedef struct {
    vec3_t position_ned;
    float accept_radius_m;
    float altitude_accept_m;
} waypoint_t;

void waypoint_init(waypoint_t *waypoint, vec3_t position_ned, float accept_radius_m);

/** Convert a global waypoint to local NED using the mission reference. */
int waypoint_from_geo(waypoint_t *waypoint, const geo_point_t *point,
                      const geo_reference_t *reference, float accept_radius_m);

float waypoint_horizontal_distance(const waypoint_t *waypoint, const vec3_t *position_ned);

/** True when both the horizontal radius and vertical acceptance are met. */
bool waypoint_reached(const waypoint_t *waypoint, const vec3_t *position_ned);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_NAVIGATOR_WAYPOINT_H */
