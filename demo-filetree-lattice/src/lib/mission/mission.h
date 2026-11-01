#ifndef LIB_MISSION_MISSION_H
#define LIB_MISSION_MISSION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MISSION_MAX_ITEMS 64
#define MISSION_DEFAULT_ACCEPT_RADIUS_M 2.0f

/** Subset of MAVLink mission commands supported by the scaffold. */
typedef enum {
    MISSION_CMD_WAYPOINT = 16,
    MISSION_CMD_LOITER_UNLIMITED = 17,
    MISSION_CMD_LOITER_TIME = 19,
    MISSION_CMD_RETURN_TO_LAUNCH = 20,
    MISSION_CMD_LAND = 21,
    MISSION_CMD_TAKEOFF = 22,
    MISSION_CMD_DELAY = 93,
} mission_command_t;

typedef struct {
    uint16_t seq;              /**< Assigned by mission_add(); must be contiguous. */
    uint16_t command;          /**< mission_command_t */
    double lat_deg;
    double lon_deg;
    float alt_m;               /**< Relative altitude, metres. */
    float hold_time_s;         /**< Loiter/hold time; 0 for waypoints. */
    float accept_radius_m;     /**< Waypoint acceptance radius. */
    uint8_t autocontinue;      /**< 1 = continue automatically. */
} mission_item_t;

typedef struct {
    mission_item_t items[MISSION_MAX_ITEMS];
    uint16_t count;
    uint16_t current;
} mission_t;

void mission_init(mission_t *mission);
void mission_clear(mission_t *mission);

/**
 * Append an item. `seq` is overwritten with the next index. A zero acceptance
 * radius is replaced with MISSION_DEFAULT_ACCEPT_RADIUS_M. Returns 0 on
 * success, -1 on invalid input, -2 if the mission is full.
 */
int mission_add(mission_t *mission, const mission_item_t *item);

/** Validate count, sequence numbers, coordinates, altitude and radius. */
int mission_validate(const mission_t *mission);

const mission_item_t *mission_current(const mission_t *mission);
uint16_t mission_count(const mission_t *mission);

/** Advance to the next item. Returns 0 on success, -1 if already at the end. */
int mission_advance(mission_t *mission);

/** Jump to an explicit sequence number. Returns 0 on success. */
int mission_set_current(mission_t *mission, uint16_t seq);

#ifdef __cplusplus
}
#endif

#endif /* LIB_MISSION_MISSION_H */
