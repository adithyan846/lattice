#include <middleware/params/param_table.h>

/*
 * Default parameter table. Keep names in sync with
 * config/boards/thejas32/default.params. Values are safe starting points, not
 * tuned flight gains.
 */

param_entry_t param_table[] = {
    {"SYS_AUTOSTART", PARAM_TYPE_INT32, {.i = 4001}},
    {"COM_ARM_AUTH", PARAM_TYPE_INT32, {.i = 1}},
    {"COM_ARM_MAG", PARAM_TYPE_INT32, {.i = 1}},
    {"COM_ARM_GNSS", PARAM_TYPE_INT32, {.i = 0}},
    {"COM_RC_LOSS_TIMEOUT_MS", PARAM_TYPE_INT32, {.i = 500}},
    {"COM_GCS_LOSS_TIMEOUT_MS", PARAM_TYPE_INT32, {.i = 5000}},
    {"COM_GNSS_LOSS_TIMEOUT_MS", PARAM_TYPE_INT32, {.i = 2000}},
    {"COM_DISARM_LAND", PARAM_TYPE_INT32, {.i = 2}},

    {"BAT_N_CELLS", PARAM_TYPE_INT32, {.i = 4}},
    {"BAT_LOW_VOLTAGE", PARAM_TYPE_FLOAT, {.f = 3.5f}},
    {"BAT_CRIT_VOLTAGE", PARAM_TYPE_FLOAT, {.f = 3.3f}},
    {"BAT_ARM_MIN", PARAM_TYPE_FLOAT, {.f = 3.6f}},

    {"MC_ROLL_P", PARAM_TYPE_FLOAT, {.f = 6.0f}},
    {"MC_PITCH_P", PARAM_TYPE_FLOAT, {.f = 6.0f}},
    {"MC_YAW_P", PARAM_TYPE_FLOAT, {.f = 2.0f}},
    {"MC_ROLLRATE_P", PARAM_TYPE_FLOAT, {.f = 0.15f}},
    {"MC_ROLLRATE_I", PARAM_TYPE_FLOAT, {.f = 0.2f}},
    {"MC_ROLLRATE_D", PARAM_TYPE_FLOAT, {.f = 0.003f}},
    {"MC_PITCHRATE_P", PARAM_TYPE_FLOAT, {.f = 0.15f}},
    {"MC_PITCHRATE_I", PARAM_TYPE_FLOAT, {.f = 0.2f}},
    {"MC_PITCHRATE_D", PARAM_TYPE_FLOAT, {.f = 0.003f}},
    {"MC_YAWRATE_P", PARAM_TYPE_FLOAT, {.f = 0.2f}},
    {"MC_YAWRATE_I", PARAM_TYPE_FLOAT, {.f = 0.1f}},
    {"MC_YAWRATE_D", PARAM_TYPE_FLOAT, {.f = 0.0f}},

    {"MPC_XY_P", PARAM_TYPE_FLOAT, {.f = 0.95f}},
    {"MPC_Z_P", PARAM_TYPE_FLOAT, {.f = 1.0f}},
    {"MPC_XY_VEL_MAX", PARAM_TYPE_FLOAT, {.f = 5.0f}},
    {"MPC_Z_VEL_MAX_UP", PARAM_TYPE_FLOAT, {.f = 3.0f}},
    {"MPC_Z_VEL_MAX_DN", PARAM_TYPE_FLOAT, {.f = 1.5f}},
    {"MPC_LAND_SPEED", PARAM_TYPE_FLOAT, {.f = 0.5f}},

    {"NAV_ACC_RAD", PARAM_TYPE_FLOAT, {.f = 2.0f}},
    {"NAV_LOITER_TIME", PARAM_TYPE_FLOAT, {.f = 0.0f}},
    {"RTL_RETURN_ALT", PARAM_TYPE_FLOAT, {.f = 30.0f}},
    {"RTL_DESCEND_ALT", PARAM_TYPE_FLOAT, {.f = 10.0f}},
    {"RTL_LAND_ENABLE", PARAM_TYPE_INT32, {.i = 1}},

    {"MAV_SYS_ID", PARAM_TYPE_INT32, {.i = 1}},
    {"MAV_COMP_ID", PARAM_TYPE_INT32, {.i = 1}},
    {"MAV_BAUD", PARAM_TYPE_INT32, {.i = 57600}},

    {"LOG_LEVEL", PARAM_TYPE_INT32, {.i = 4}},
};

const size_t param_table_count = sizeof(param_table) / sizeof(param_table[0]);
