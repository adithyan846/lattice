#include <modules/navigator/navigator_mission.h>

#include <stddef.h>
#include <string.h>

#include <modules/navigator/waypoint.h>

static int item_to_local(const navigator_mission_t *navigator, const mission_item_t *item,
                         vec3_t *target)
{
    if (navigator == NULL || item == NULL || target == NULL || !navigator->reference_valid) {
        return -1;
    }

    const geo_point_t point = {.lat_deg = item->lat_deg, .lon_deg = item->lon_deg, .alt_m = item->alt_m};
    geo_global_to_local(&point, &navigator->reference, target);
    return 0;
}

static navigator_mission_event_t advance(navigator_mission_t *navigator)
{
    navigator->hold_elapsed_s = 0.0f;
    navigator->reached_reported = false;

    if (mission_advance(&navigator->mission) != 0) {
        navigator->active = false;
        return NAVIGATOR_MISSION_EVENT_COMPLETE;
    }

    navigator->current_seq = navigator->mission.current;
    return NAVIGATOR_MISSION_EVENT_ITEM_REACHED;
}

void navigator_mission_init(navigator_mission_t *navigator)
{
    if (navigator == NULL) {
        return;
    }

    memset(navigator, 0, sizeof(*navigator));
    mission_init(&navigator->mission);
}

int navigator_mission_load(navigator_mission_t *navigator, const mission_t *mission)
{
    if (navigator == NULL || mission == NULL || mission_validate(mission) != 0) {
        return -1;
    }

    navigator->mission = *mission;
    navigator->mission.current = 0;
    navigator->current_seq = 0;
    navigator->hold_elapsed_s = 0.0f;
    navigator->reached_reported = false;
    navigator->active = false;
    navigator->paused = false;
    return 0;
}

int navigator_mission_set_reference(navigator_mission_t *navigator, const geo_reference_t *reference)
{
    if (navigator == NULL || reference == NULL) {
        return -1;
    }

    navigator->reference = *reference;
    navigator->reference_valid = true;
    return 0;
}

int navigator_mission_start(navigator_mission_t *navigator)
{
    if (navigator == NULL || mission_validate(&navigator->mission) != 0 ||
        !navigator->reference_valid) {
        return -1;
    }

    navigator->mission.current = 0;
    navigator->current_seq = 0;
    navigator->hold_elapsed_s = 0.0f;
    navigator->reached_reported = false;
    navigator->paused = false;
    navigator->active = true;
    navigator->target_valid = false;
    return 0;
}

void navigator_mission_pause(navigator_mission_t *navigator, bool paused)
{
    if (navigator != NULL) {
        navigator->paused = paused;
    }
}

void navigator_mission_abort(navigator_mission_t *navigator)
{
    if (navigator != NULL) {
        navigator->active = false;
        navigator->target_valid = false;
    }
}

navigator_mission_event_t navigator_mission_update(navigator_mission_t *navigator,
                                                   const vec3_t *position_ned, float dt_s)
{
    if (navigator == NULL || position_ned == NULL || dt_s < 0.0f) {
        return NAVIGATOR_MISSION_EVENT_NONE;
    }

    navigator->position_ned = *position_ned;

    if (!navigator->active || navigator->paused) {
        return NAVIGATOR_MISSION_EVENT_NONE;
    }

    const mission_item_t *item = mission_current(&navigator->mission);

    if (item == NULL) {
        navigator->active = false;
        return NAVIGATOR_MISSION_EVENT_COMPLETE;
    }

    switch (item->command) {
    case MISSION_CMD_LAND:
        navigator->target_ned = *position_ned;
        navigator->target_valid = true;
        return NAVIGATOR_MISSION_EVENT_LAND;

    case MISSION_CMD_RETURN_TO_LAUNCH:
        navigator->target_valid = false;
        return NAVIGATOR_MISSION_EVENT_RTL;

    case MISSION_CMD_DELAY:
        navigator->target_ned = *position_ned;
        navigator->target_valid = true;
        navigator->hold_elapsed_s += dt_s;

        if (navigator->hold_elapsed_s >= item->hold_time_s) {
            return advance(navigator);
        }

        return NAVIGATOR_MISSION_EVENT_NONE;

    case MISSION_CMD_WAYPOINT:
    case MISSION_CMD_TAKEOFF:
    case MISSION_CMD_LOITER_TIME:
    case MISSION_CMD_LOITER_UNLIMITED:
    default:
        break;
    }

    vec3_t target;

    if (item_to_local(navigator, item, &target) != 0) {
        navigator->active = false;
        return NAVIGATOR_MISSION_EVENT_COMPLETE;
    }

    navigator->target_ned = target;
    navigator->target_valid = true;

    const float accept_radius =
        item->accept_radius_m > 0.0f ? item->accept_radius_m : MISSION_DEFAULT_ACCEPT_RADIUS_M;
    waypoint_t waypoint;
    waypoint_init(&waypoint, target, accept_radius);
    const bool reached = waypoint_reached(&waypoint, position_ned);

    if (!reached) {
        return NAVIGATOR_MISSION_EVENT_NONE;
    }

    if (!navigator->reached_reported) {
        navigator->reached_reported = true;
        navigator->hold_elapsed_s = 0.0f;
        return NAVIGATOR_MISSION_EVENT_ITEM_REACHED;
    }

    if (item->command == MISSION_CMD_LOITER_UNLIMITED) {
        return NAVIGATOR_MISSION_EVENT_NONE;
    }

    if (item->command == MISSION_CMD_LOITER_TIME) {
        navigator->hold_elapsed_s += dt_s;

        if (navigator->hold_elapsed_s < item->hold_time_s) {
            return NAVIGATOR_MISSION_EVENT_NONE;
        }
    }

    return advance(navigator);
}

const vec3_t *navigator_mission_target(const navigator_mission_t *navigator)
{
    if (navigator == NULL || !navigator->target_valid) {
        return NULL;
    }

    return &navigator->target_ned;
}

uint16_t navigator_mission_current_seq(const navigator_mission_t *navigator)
{
    return navigator == NULL ? 0 : navigator->current_seq;
}

uint16_t navigator_mission_total(const navigator_mission_t *navigator)
{
    return navigator == NULL ? 0 : navigator->mission.count;
}
