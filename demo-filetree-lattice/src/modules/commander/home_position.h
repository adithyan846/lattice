#ifndef MODULES_COMMANDER_HOME_POSITION_H
#define MODULES_COMMANDER_HOME_POSITION_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double lat_deg;
    double lon_deg;
    float alt_m;
    bool valid;
} home_position_t;

void home_position_init(home_position_t *home);
int home_position_set(home_position_t *home, double lat_deg, double lon_deg, float alt_m);
void home_position_clear(home_position_t *home);
bool home_position_valid(const home_position_t *home);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_COMMANDER_HOME_POSITION_H */
