#include <modules/commander/home_position.h>

#include <stddef.h>

void home_position_init(home_position_t *home)
{
    if (home == NULL) {
        return;
    }

    home->lat_deg = 0.0;
    home->lon_deg = 0.0;
    home->alt_m = 0.0f;
    home->valid = false;
}

int home_position_set(home_position_t *home, double lat_deg, double lon_deg, float alt_m)
{
    if (home == NULL || lat_deg < -90.0 || lat_deg > 90.0 || lon_deg < -180.0 ||
        lon_deg > 180.0) {
        return -1;
    }

    home->lat_deg = lat_deg;
    home->lon_deg = lon_deg;
    home->alt_m = alt_m;
    home->valid = true;
    return 0;
}

void home_position_clear(home_position_t *home)
{
    if (home != NULL) {
        home->valid = false;
    }
}

bool home_position_valid(const home_position_t *home)
{
    return home != NULL && home->valid;
}
