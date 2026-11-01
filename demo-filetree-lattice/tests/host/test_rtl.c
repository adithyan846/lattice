#include "test_assert.h"

#include <lib/rtl/rtl.h>

void test_rtl(void)
{
    rtl_profile_t rtl;
    rtl_init(&rtl, 30.0f, 10.0f);

    const vec3_t home = {0.0f, 0.0f, 0.0f};

    /* Below return altitude and away from home -> climb. */
    vec3_t position = {100.0f, 0.0f, -10.0f};
    TEST_ASSERT(rtl_update(&rtl, &position, &home, true) == RTL_PHASE_CLIMB);

    /* At return altitude but far from home -> cruise. */
    position = vec3_make(100.0f, 0.0f, -31.0f);
    TEST_ASSERT(rtl_update(&rtl, &position, &home, true) == RTL_PHASE_CRUISE);

    /* Above home and above descend altitude -> descend. */
    position = vec3_make(0.0f, 0.0f, -11.0f);
    TEST_ASSERT(rtl_update(&rtl, &position, &home, true) == RTL_PHASE_DESCEND);

    /* Close to ground -> land. */
    position = vec3_make(0.0f, 0.0f, -5.0f);
    TEST_ASSERT(rtl_update(&rtl, &position, &home, true) == RTL_PHASE_LAND);

    /* On the ground -> done. */
    position = vec3_make(0.0f, 0.0f, 0.0f);
    TEST_ASSERT(rtl_update(&rtl, &position, &home, true) == RTL_PHASE_DONE);

    /* Invalid home -> idle. */
    TEST_ASSERT(rtl_update(&rtl, &position, &home, false) == RTL_PHASE_IDLE);
}
