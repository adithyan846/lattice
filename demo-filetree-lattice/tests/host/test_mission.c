#include "test_assert.h"

#include <lib/mission/mission.h>

#include <string.h>

static mission_item_t waypoint(uint16_t command, double lat, double lon, float alt)
{
    mission_item_t item;
    memset(&item, 0, sizeof(item));
    item.command = command;
    item.lat_deg = lat;
    item.lon_deg = lon;
    item.alt_m = alt;
    item.accept_radius_m = 2.0f;
    item.autocontinue = 1;
    return item;
}

void test_mission(void)
{
    mission_t mission;
    mission_init(&mission);

    TEST_ASSERT(mission_count(&mission) == 0);
    TEST_ASSERT(mission_current(&mission) == NULL);
    TEST_ASSERT(mission_validate(&mission) != 0);

    const mission_item_t takeoff = waypoint(MISSION_CMD_TAKEOFF, 12.9716, 77.5946, 10.0f);
    const mission_item_t wp1 = waypoint(MISSION_CMD_WAYPOINT, 12.9720, 77.5950, 30.0f);
    const mission_item_t rtl = waypoint(MISSION_CMD_RETURN_TO_LAUNCH, 12.9716, 77.5946, 30.0f);

    TEST_ASSERT(mission_add(&mission, &takeoff) == 0);
    TEST_ASSERT(mission_add(&mission, &wp1) == 0);
    TEST_ASSERT(mission_add(&mission, &rtl) == 0);
    TEST_ASSERT(mission_count(&mission) == 3);
    TEST_ASSERT(mission.items[0].seq == 0);
    TEST_ASSERT(mission.items[1].seq == 1);
    TEST_ASSERT(mission.items[2].seq == 2);
    TEST_ASSERT(mission_validate(&mission) == 0);

    const mission_item_t *current = mission_current(&mission);
    TEST_ASSERT(current != NULL);
    TEST_ASSERT(current->seq == 0);
    TEST_ASSERT(current->command == MISSION_CMD_TAKEOFF);

    TEST_ASSERT(mission_advance(&mission) == 0);
    TEST_ASSERT(mission_current(&mission)->seq == 1);
    TEST_ASSERT(mission_set_current(&mission, 2) == 0);
    TEST_ASSERT(mission_current(&mission)->seq == 2);
    TEST_ASSERT(mission_advance(&mission) != 0);

    /* Invalid latitude is rejected. */
    const mission_item_t bad = waypoint(MISSION_CMD_WAYPOINT, 100.0, 77.0, 30.0f);
    TEST_ASSERT(mission_add(&mission, &bad) != 0);

    /* Unsupported command is rejected. */
    const mission_item_t unsupported = waypoint(999, 12.0, 77.0, 30.0f);
    TEST_ASSERT(mission_add(&mission, &unsupported) != 0);

    /* Default acceptance radius is applied. */
    mission_item_t no_radius = waypoint(MISSION_CMD_WAYPOINT, 12.0, 77.0, 30.0f);
    no_radius.accept_radius_m = 0.0f;
    mission_init(&mission);
    TEST_ASSERT(mission_add(&mission, &no_radius) == 0);
    TEST_ASSERT_NEAR(mission.items[0].accept_radius_m, MISSION_DEFAULT_ACCEPT_RADIUS_M, 1e-6);
}
