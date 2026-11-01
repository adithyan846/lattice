#include <lib/mission/mission.h>

#include <stddef.h>
#include <string.h>

static int mission_command_supported(uint16_t command)
{
    switch (command) {
    case MISSION_CMD_WAYPOINT:
    case MISSION_CMD_LOITER_UNLIMITED:
    case MISSION_CMD_LOITER_TIME:
    case MISSION_CMD_RETURN_TO_LAUNCH:
    case MISSION_CMD_LAND:
    case MISSION_CMD_TAKEOFF:
    case MISSION_CMD_DELAY:
        return 1;

    default:
        return 0;
    }
}

static int mission_item_valid(const mission_item_t *item)
{
    if (item == NULL) {
        return 0;
    }

    if (!mission_command_supported(item->command)) {
        return 0;
    }

    if (item->lat_deg < -90.0 || item->lat_deg > 90.0) {
        return 0;
    }

    if (item->lon_deg < -180.0 || item->lon_deg > 180.0) {
        return 0;
    }

    if (item->alt_m < -500.0f || item->alt_m > 10000.0f) {
        return 0;
    }

    if (item->accept_radius_m < 0.0f) {
        return 0;
    }

    return 1;
}

void mission_init(mission_t *mission)
{
    if (mission == NULL) {
        return;
    }

    memset(mission, 0, sizeof(*mission));
}

void mission_clear(mission_t *mission)
{
    mission_init(mission);
}

int mission_add(mission_t *mission, const mission_item_t *item)
{
    if (mission == NULL || item == NULL) {
        return -1;
    }

    if (mission->count >= MISSION_MAX_ITEMS) {
        return -2;
    }

    mission_item_t copy = *item;
    copy.seq = mission->count;

    if (copy.accept_radius_m <= 0.0f) {
        copy.accept_radius_m = MISSION_DEFAULT_ACCEPT_RADIUS_M;
    }

    if (!mission_item_valid(&copy)) {
        return -1;
    }

    mission->items[mission->count] = copy;
    mission->count++;
    return 0;
}

int mission_validate(const mission_t *mission)
{
    if (mission == NULL || mission->count == 0 || mission->count > MISSION_MAX_ITEMS) {
        return -1;
    }

    for (uint16_t i = 0; i < mission->count; i++) {
        const mission_item_t *item = &mission->items[i];

        if (item->seq != i) {
            return -1;
        }

        if (!mission_item_valid(item)) {
            return -1;
        }
    }

    return 0;
}

const mission_item_t *mission_current(const mission_t *mission)
{
    if (mission == NULL || mission->count == 0 || mission->current >= mission->count) {
        return NULL;
    }

    return &mission->items[mission->current];
}

uint16_t mission_count(const mission_t *mission)
{
    return mission == NULL ? 0 : mission->count;
}

int mission_advance(mission_t *mission)
{
    if (mission == NULL || mission->count == 0) {
        return -1;
    }

    if (mission->current + 1 >= mission->count) {
        return -1;
    }

    mission->current++;
    return 0;
}

int mission_set_current(mission_t *mission, uint16_t seq)
{
    if (mission == NULL || seq >= mission->count) {
        return -1;
    }

    mission->current = seq;
    return 0;
}
