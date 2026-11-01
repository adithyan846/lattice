#include <modules/navigator/waypoint.h>

#include <lib/geo/geo.h>
#include <lib/math/vec3.h>

#include "test_assert.h"

void test_waypoint(void)
{
    waypoint_t waypoint;
    waypoint_init(&waypoint, vec3_make(10.0f, 0.0f, -20.0f), 2.0f);

    const vec3_t at_target = vec3_make(10.0f, 0.0f, -20.0f);
    TEST_ASSERT(waypoint_reached(&waypoint, &at_target));

    const vec3_t slightly_off = vec3_make(11.0f, 0.0f, -20.5f);
    TEST_ASSERT(waypoint_reached(&waypoint, &slightly_off));

    const vec3_t too_far = vec3_make(13.0f, 0.0f, -20.0f);
    TEST_ASSERT(!waypoint_reached(&waypoint, &too_far));

    const vec3_t too_high = vec3_make(10.0f, 0.0f, -25.0f);
    TEST_ASSERT(!waypoint_reached(&waypoint, &too_high));

    const geo_reference_t reference = {.lat_deg = 47.0, .lon_deg = 8.0, .alt_m = 400.0f};
    const geo_point_t point = {.lat_deg = 47.0001, .lon_deg = 8.0, .alt_m = 420.0f};
    TEST_ASSERT(waypoint_from_geo(&waypoint, &point, &reference, 2.0f) == 0);
    TEST_ASSERT(waypoint.position_ned.z < -19.0f);
}
