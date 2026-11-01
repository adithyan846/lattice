#include <modules/navigator/waypoint.h>

#include <math.h>
#include <stddef.h>

#include <lib/geo/geo.h>

void waypoint_init(waypoint_t *waypoint, vec3_t position_ned, float accept_radius_m)
{
    if (waypoint == NULL) {
        return;
    }

    waypoint->position_ned = position_ned;
    waypoint->accept_radius_m = accept_radius_m > 0.0f ? accept_radius_m : 2.0f;
    waypoint->altitude_accept_m = WAYPOINT_DEFAULT_ALT_ACCEPT_M;
}

int waypoint_from_geo(waypoint_t *waypoint, const geo_point_t *point,
                      const geo_reference_t *reference, float accept_radius_m)
{
    if (waypoint == NULL || point == NULL || reference == NULL) {
        return -1;
    }

    vec3_t local;
    geo_global_to_local(point, reference, &local);
    waypoint_init(waypoint, local, accept_radius_m);
    return 0;
}

float waypoint_horizontal_distance(const waypoint_t *waypoint, const vec3_t *position_ned)
{
    if (waypoint == NULL || position_ned == NULL) {
        return 0.0f;
    }

    const float dx = waypoint->position_ned.x - position_ned->x;
    const float dy = waypoint->position_ned.y - position_ned->y;
    return sqrtf(dx * dx + dy * dy);
}

bool waypoint_reached(const waypoint_t *waypoint, const vec3_t *position_ned)
{
    if (waypoint == NULL || position_ned == NULL) {
        return false;
    }

    const float horizontal = waypoint_horizontal_distance(waypoint, position_ned);
    const float vertical = fabsf(waypoint->position_ned.z - position_ned->z);

    return horizontal <= waypoint->accept_radius_m && vertical <= waypoint->altitude_accept_m;
}
