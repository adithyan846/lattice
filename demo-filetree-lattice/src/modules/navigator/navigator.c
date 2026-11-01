#include <modules/navigator/navigator.h>

#include <stddef.h>
#include <string.h>

#include <middleware/log/log.h>

void navigator_init(navigator_t *navigator)
{
    if (navigator == NULL) {
        return;
    }

    memset(navigator, 0, sizeof(*navigator));
    navigator_mission_init(&navigator->mission);
    navigator_rtl_init(&navigator->rtl, 30.0f, 5.0f);
    navigator->state = NAVIGATOR_STATE_IDLE;
}

int navigator_load_mission(navigator_t *navigator, const mission_t *mission)
{
    if (navigator == NULL) {
        return -1;
    }

    if (navigator_mission_load(&navigator->mission, mission) != 0) {
        return -1;
    }

    navigator->state = NAVIGATOR_STATE_IDLE;
    navigator->target_valid = false;
    return 0;
}

int navigator_set_reference(navigator_t *navigator, const geo_reference_t *reference)
{
    if (navigator == NULL) {
        return -1;
    }

    return navigator_mission_set_reference(&navigator->mission, reference);
}

int navigator_start_mission(navigator_t *navigator)
{
    if (navigator == NULL || navigator_mission_start(&navigator->mission) != 0) {
        return -1;
    }

    navigator->state = NAVIGATOR_STATE_MISSION;
    navigator->target_valid = false;
    return 0;
}

void navigator_pause_mission(navigator_t *navigator, bool paused)
{
    if (navigator != NULL) {
        navigator_mission_pause(&navigator->mission, paused);
    }
}

int navigator_start_rtl(navigator_t *navigator, const vec3_t *home_ned)
{
    if (navigator == NULL || navigator_rtl_start(&navigator->rtl, home_ned) != 0) {
        return -1;
    }

    navigator->state = NAVIGATOR_STATE_RTL;
    navigator->target_valid = false;
    return 0;
}

int navigator_start_land(navigator_t *navigator)
{
    if (navigator == NULL) {
        return -1;
    }

    navigator->state = NAVIGATOR_STATE_LAND;
    navigator->target_valid = false;
    return 0;
}

void navigator_abort(navigator_t *navigator)
{
    if (navigator == NULL) {
        return;
    }

    navigator_mission_abort(&navigator->mission);
    navigator_rtl_stop(&navigator->rtl);
    navigator->state = NAVIGATOR_STATE_IDLE;
    navigator->target_valid = false;
}

navigator_state_t navigator_state(const navigator_t *navigator)
{
    return navigator == NULL ? NAVIGATOR_STATE_IDLE : navigator->state;
}

const vec3_t *navigator_target(const navigator_t *navigator)
{
    if (navigator == NULL || !navigator->target_valid) {
        return NULL;
    }

    return &navigator->target_ned;
}

uint16_t navigator_current_seq(const navigator_t *navigator)
{
    return navigator == NULL ? 0 : navigator_mission_current_seq(&navigator->mission);
}

uint16_t navigator_total_items(const navigator_t *navigator)
{
    return navigator == NULL ? 0 : navigator_mission_total(&navigator->mission);
}

navigator_event_t navigator_update(navigator_t *navigator, const vec3_t *position_ned, float dt_s)
{
    if (navigator == NULL || position_ned == NULL || dt_s < 0.0f) {
        return NAVIGATOR_EVENT_NONE;
    }

    switch (navigator->state) {
    case NAVIGATOR_STATE_MISSION: {
        const navigator_mission_event_t event =
            navigator_mission_update(&navigator->mission, position_ned, dt_s);
        const vec3_t *target = navigator_mission_target(&navigator->mission);

        navigator->target_valid = target != NULL;

        if (target != NULL) {
            navigator->target_ned = *target;
        }

        switch (event) {
        case NAVIGATOR_MISSION_EVENT_ITEM_REACHED:
            return NAVIGATOR_EVENT_ITEM_REACHED;

        case NAVIGATOR_MISSION_EVENT_COMPLETE:
            navigator->state = NAVIGATOR_STATE_IDLE;
            navigator->target_valid = false;
            return NAVIGATOR_EVENT_MISSION_COMPLETE;

        case NAVIGATOR_MISSION_EVENT_LAND:
            navigator_start_land(navigator);
            return NAVIGATOR_EVENT_NONE;

        case NAVIGATOR_MISSION_EVENT_RTL:
            return NAVIGATOR_EVENT_NONE;

        case NAVIGATOR_MISSION_EVENT_NONE:
        default:
            return NAVIGATOR_EVENT_NONE;
        }
    }

    case NAVIGATOR_STATE_RTL: {
        const rtl_phase_t phase = navigator_rtl_update(&navigator->rtl, position_ned);

        if (phase == RTL_PHASE_IDLE) {
            navigator->target_valid = false;
            return NAVIGATOR_EVENT_NONE;
        }

        const vec3_t *target = navigator_rtl_target(&navigator->rtl);

        navigator->target_valid = target != NULL;

        if (target != NULL) {
            navigator->target_ned = *target;
        }

        if (phase == RTL_PHASE_DONE) {
            navigator->state = NAVIGATOR_STATE_IDLE;
            return NAVIGATOR_EVENT_RTL_COMPLETE;
        }

        return NAVIGATOR_EVENT_NONE;
    }

    case NAVIGATOR_STATE_LAND:
        navigator->target_ned = *position_ned;
        navigator->target_ned.z = navigator->land_z_m;
        navigator->target_valid = true;

        if (position_ned->z >= navigator->land_z_m - 0.2f) {
            navigator->state = NAVIGATOR_STATE_IDLE;
            return NAVIGATOR_EVENT_LAND_REACHED;
        }

        return NAVIGATOR_EVENT_NONE;

    case NAVIGATOR_STATE_IDLE:
    default:
        navigator->target_valid = false;
        return NAVIGATOR_EVENT_NONE;
    }
}

int navigator_start(void)
{
    /* TODO: create the navigator task, subscribe to vehicle_local_position,
     * home_position, vehicle_command and mission_item, and publish
     * mission_current plus trajectory_setpoint. */
    FRT_LOG_WARN("navigator: scaffold only, navigation task not implemented");
    return -1;
}

int navigator_stop(void)
{
    return 0;
}
