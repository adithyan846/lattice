#include <stddef.h>

#include <freertrtos.h>
#include <middleware/log/log.h>

#include <main/app_config.h>
#include <main/init.h>
#include <main/version.h>

int main(void)
{
    if (app_init() != 0) {
        FRT_LOG_ERROR("boot: initialisation failed");
        frt_start_scheduler();
        return -1;
    }

    FRT_LOG_INFO("boot: %s %s starting", APP_NAME, app_version());
    (void)app_start_modules();
    frt_start_scheduler();

    /* The scheduler does not return on success. */
    app_shutdown();
    return 0;
}
