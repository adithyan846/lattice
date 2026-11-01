#include "test_assert.h"

#include <lib/geo/geo.h>

void test_geo(void)
{
    const geo_reference_t ref = {12.9716, 77.5946, 920.0f};
    const geo_point_t ref_point = {12.9716, 77.5946, 920.0f};

    /* One degree of latitude is about 111.2 km. */
    const geo_point_t north = {13.9716, 77.5946, 920.0f};
    vec3_t local = {0};
    geo_global_to_local(&north, &ref, &local);
    TEST_ASSERT_NEAR(local.x, 111194.9f, 200.0f);
    TEST_ASSERT_NEAR(local.y, 0.0f, 1.0f);
    TEST_ASSERT_NEAR(local.z, 0.0f, 1.0f);

    /* Round-trip a local offset. */
    const vec3_t offset = {123.0f, -456.0f, -30.0f};
    geo_point_t global = {0};
    geo_local_to_global(&offset, &ref, &global);
    vec3_t round_trip = {0};
    geo_global_to_local(&global, &ref, &round_trip);
    TEST_ASSERT_NEAR(round_trip.x, offset.x, 0.1f);
    TEST_ASSERT_NEAR(round_trip.y, offset.y, 0.1f);
    TEST_ASSERT_NEAR(round_trip.z, offset.z, 0.1f);

    /* Distance and bearing. */
    const geo_point_t east = {12.9716, 78.5946, 920.0f};
    TEST_ASSERT(geo_distance_m(&ref_point, &north) > 110000.0f);
    TEST_ASSERT(geo_distance_m(&ref_point, &north) < 112000.0f);
    TEST_ASSERT_NEAR(geo_bearing_deg(&ref_point, &north), 0.0f, 0.5f);
    TEST_ASSERT_NEAR(geo_bearing_deg(&ref_point, &east), 90.0f, 2.0f);

    /* Identical points have zero distance. */
    TEST_ASSERT_NEAR(geo_distance_m(&ref_point, &ref_point), 0.0f, 1e-3);
}
