#ifndef MODULES_NAVIGATOR_NAVIGATOR_MISSION_H
#define MODULES_NAVIGATOR_NAVIGATOR_MISSION_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/geo/geo.h>
#include <lib/math/vec3.h>
#include <lib/mission/mission.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NAVIGATOR_MISSION_EVENT_NONE = 0,
    NAVIGATOR_MISSION_EVENT_ITEM_REACHED,
    NAVIGATOR_MISSION_EVENT_COMPLETE,
    NAVIGATOR_MISSION_EVENT_LAND,
    NAVIGATOR_MISSION_EVENT_RTL,
} navigator_mission_event_t;

typedef struct {
    mission_t mission;
    geo_reference_t reference;
    vec3_t target_ned;
    vec3_t position_ned;
    float hold_elapsed_s;
    uint16_t current_seq;
    bool reference_valid;
    bool active;
    bool paused;
    bool reached_reported;
    bool target_valid;
} navigator_mission_t;

void navigator_mission_init(navigator_mission_t *navigator);

/** Copy and validate a mission. Returns 0 on success, -1 if invalid. */
int navigator_mission_load(navigator_mission_t *navigator, const mission_t *mission);

/** Set the local tangent-plane origin used to convert waypoints to NED. */
int navigator_mission_set_reference(navigator_mission_t *navigator, const geo_reference_t *reference);

int navigator_mission_start(navigator_mission_t *navigator);
void navigator_mission_pause(navigator_mission_t *navigator, bool paused);
void navigator_mission_abort(navigator_mission_t *navigator);

/**
 * Advance mission sequencing. The current target is written to
 * `navigator->target_ned`. Returns the event for this step.
 *
 * The caller is responsible for altitude/position control; this function only
 * decides when an item is reached and which item is next.
 */
navigator_mission_event_t navigator_mission_update(navigator_mission_t *navigator,
                                                   const vec3_t *position_ned, float dt_s);

const vec3_t *navigator_mission_target(const navigator_mission_t *navigator);
uint16_t navigator_mission_current_seq(const navigator_mission_t *navigator);
uint16_t navigator_mission_total(const navigator_mission_t *navigator);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_NAVIGATOR_NAVIGATOR_MISSION_H */
