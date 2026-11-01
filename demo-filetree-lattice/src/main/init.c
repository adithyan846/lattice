#include <main/init.h>

#include <stddef.h>

#include <board.h>
#include <freertrtos.h>
#include <middleware/log/log.h>
#include <middleware/params/params.h>
#include <middleware/time/time.h>
#include <middleware/uorb/uorb.h>

#include <main/task_registry.h>
#include <main/app_config.h>
#include <main/version.h>
#include <modules/battery_status/battery_status.h>
#include <modules/commander/commander.h>
#include <modules/land_detector/land_detector.h>
#include <modules/logger/logger.h>
#include <modules/mavlink/mavlink.h>
#include <modules/navigator/navigator.h>
#include <modules/rc_update/rc_update.h>
#include <modules/sensors/sensors.h>
#include <modules/state_estimator/state_estimator.h>
#include <modules/task_watchdog/task_watchdog.h>

typedef struct {
    const char *name;
    int (*start)(void);
    int (*stop)(void);
} module_entry_t;

static const module_entry_t modules[] = {
    {"sensors", sensors_start, sensors_stop},
    {"state_estimator", state_estimator_start, state_estimator_stop},
    {"commander", commander_start, commander_stop},
    {"navigator", navigator_start, navigator_stop},
    {"mavlink", mavlink_start, mavlink_stop},
    {"logger", logger_start, logger_stop},
    {"battery_status", battery_status_start, battery_status_stop},
    {"rc_update", rc_update_start, rc_update_stop},
    {"land_detector", land_detector_start, land_detector_stop},
    {"task_watchdog", task_watchdog_start, task_watchdog_stop},
};

int app_init(void)
{
    if (board_init() != 0) {
        return -1;
    }

    if (frt_init() != 0) {
        return -1;
    }

    hrt_init();

    if (uorb_init() != 0) {
        return -1;
    }

    if (params_init() != 0) {
        return -1;
    }

    if (params_load() != 0) {
        FRT_LOG_WARN("params: no stored parameter set, using defaults");
    }

    task_registry_init();
    FRT_LOG_INFO("boot: %s (%s)", APP_NAME, app_version());
    return 0;
}

int app_start_modules(void)
{
    int started = 0;

    for (size_t index = 0; index < sizeof(modules) / sizeof(modules[0]); index++) {
        if (modules[index].start() == 0) {
            started++;

        } else {
            FRT_LOG_WARN("boot: %s did not start (scaffold)", modules[index].name);
        }
    }

    if (started == 0) {
        FRT_LOG_ERROR("boot: no flight modules started; autopilot is not operational");
    }

    return started;
}

void app_shutdown(void)
{
    for (size_t index = sizeof(modules) / sizeof(modules[0]); index > 0; index--) {
        if (modules[index - 1].stop != NULL) {
            (void)modules[index - 1].stop();
        }
    }
}
