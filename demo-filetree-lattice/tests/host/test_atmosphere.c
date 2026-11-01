#include "test_assert.h"

#include <lib/atmosphere/atmosphere.h>

void test_atmosphere(void)
{
    TEST_ASSERT_NEAR(atmosphere_altitude_from_pressure(ATMOSPHERE_SEA_LEVEL_PRESSURE_PA,
                                                       ATMOSPHERE_SEA_LEVEL_PRESSURE_PA),
                     0.0f, 0.01f);

    const float pressure_30m = atmosphere_pressure_from_altitude(30.0f,
                                                                 ATMOSPHERE_SEA_LEVEL_PRESSURE_PA);
    TEST_ASSERT_NEAR(atmosphere_altitude_from_pressure(pressure_30m,
                                                       ATMOSPHERE_SEA_LEVEL_PRESSURE_PA),
                     30.0f, 0.1f);

    const float pressure_1000m = atmosphere_pressure_from_altitude(1000.0f,
                                                                   ATMOSPHERE_SEA_LEVEL_PRESSURE_PA);
    TEST_ASSERT_NEAR(pressure_1000m, 89874.6f, 10.0f);

    /* Invalid pressure must not divide by zero. */
    TEST_ASSERT_NEAR(atmosphere_altitude_from_pressure(0.0f,
                                                       ATMOSPHERE_SEA_LEVEL_PRESSURE_PA),
                     0.0f, 1e-6);

    TEST_ASSERT(atmosphere_density(0.0f) > 1.1f);
    TEST_ASSERT(atmosphere_density(0.0f) < 1.3f);
    TEST_ASSERT(atmosphere_density(1000.0f) < atmosphere_density(0.0f));
}
