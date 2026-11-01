#ifndef MODULES_NAVIGATOR_NAVIGATOR_H
#define MODULES_NAVIGATOR_NAVIGATOR_H

#include <stdbool.h>

#include <lib/geo/geo.h>
#include <lib/mission/mission.h>
#include <modules/navigator/navigator_mission.h>
#include <modules/navigator/navigator_rtl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NAVIGATOR_STATE_IDLE = 0,
    NAVIGATOR_STATE_MISSION,
    NAVIGATOR_STATE_RTL,
    NAVIGATOR_STATE_LAND,
} navigator_state_t;

typedef enum {
    NAVIGATOR_EVENT_NONE = 0,
    NAVIGATOR_EVENT_ITEM_REACHED,
    NAVIGATOR_EVENT_MISSION_COMPLETE,
    NAVIGATOR_EVENT_LAND_REACHED,
    NAVIGATOR_EVENT_RTL_COMPLETE,
} navigator_event_t;

typedef struct {
    navigator_state_t state;
    navigator_mission_t mission;
    navigator_rtl_t rtl;
    vec3_t target_ned;
    float land_z_m;
    bool target_valid;
} navigator_t;

void navigator_init(navigator_t *navigator);

int navigator_load_mission(navigator_t *navigator, const mission_t *mission);
int navigator_set_reference(navigator_t *navigator, const geo_reference_t *reference);
int navigator_start_mission(navigator_t *navigator);
void navigator_pause_mission(navigator_t *navigator, bool paused);
int navigator_start_rtl(navigator_t *navigator, const vec3_t *home_ned);
int navigator_start_land(navigator_t *navigator);
void navigator_abort(navigator_t *navigator);

navigator_state_t navigator_state(const navigator_t *navigator);
const vec3_t *navigator_target(const navigator_t *navigator);
uint16_t navigator_current_seq(const navigator_t *navigator);
uint16_t navigator_total_items(const navigator_t *navigator);

/**
 * Advance the active navigation state. Returns an event for the caller (usually
 * commander) to act on, for example switching to land after a mission LAND
 * command. The output target is valid while navigator_target() is non-NULL.
 */
navigator_event_t navigator_update(navigator_t *navigator, const vec3_t *position_ned, float dt_s);

int navigator_start(void);
int navigator_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_NAVIGATOR_NAVIGATOR_H */
