#include <lib/geo/geo.h>

#include <math.h>
#include <stddef.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)

void geo_global_to_local(const geo_point_t *global, const geo_reference_t *ref, vec3_t *local_ned)
{
    if (global == NULL || ref == NULL || local_ned == NULL) {
        return;
    }

    const double dlat = (global->lat_deg - ref->lat_deg) * DEG_TO_RAD;
    const double dlon = (global->lon_deg - ref->lon_deg) * DEG_TO_RAD;
    const double mean_lat = 0.5 * (global->lat_deg + ref->lat_deg) * DEG_TO_RAD;

    local_ned->x = (float)(dlat * GEO_EARTH_RADIUS_M);
    local_ned->y = (float)(dlon * cos(mean_lat) * GEO_EARTH_RADIUS_M);
    local_ned->z = -(global->alt_m - ref->alt_m);
}

void geo_local_to_global(const vec3_t *local_ned, const geo_reference_t *ref, geo_point_t *global)
{
    if (local_ned == NULL || ref == NULL || global == NULL) {
        return;
    }

    const double ref_lat_rad = ref->lat_deg * DEG_TO_RAD;
    double cos_lat = cos(ref_lat_rad);

    if (fabs(cos_lat) < 1e-9) {
        cos_lat = 1e-9;
    }

    global->lat_deg = ref->lat_deg + (local_ned->x / GEO_EARTH_RADIUS_M) * RAD_TO_DEG;
    global->lon_deg = ref->lon_deg +
                      (local_ned->y / (GEO_EARTH_RADIUS_M * cos_lat)) * RAD_TO_DEG;
    global->alt_m = ref->alt_m - local_ned->z;
}

float geo_distance_m(const geo_point_t *a, const geo_point_t *b)
{
    if (a == NULL || b == NULL) {
        return 0.0f;
    }

    const double lat1 = a->lat_deg * DEG_TO_RAD;
    const double lat2 = b->lat_deg * DEG_TO_RAD;
    const double dlat = lat2 - lat1;
    const double dlon = (b->lon_deg - a->lon_deg) * DEG_TO_RAD;

    const double sin_dlat = sin(dlat * 0.5);
    const double sin_dlon = sin(dlon * 0.5);
    const double h = sin_dlat * sin_dlat + cos(lat1) * cos(lat2) * sin_dlon * sin_dlon;
    const double c = 2.0 * atan2(sqrt(h), sqrt(1.0 - h));

    return (float)(GEO_EARTH_RADIUS_M * c);
}

float geo_bearing_deg(const geo_point_t *from, const geo_point_t *to)
{
    if (from == NULL || to == NULL) {
        return 0.0f;
    }

    const double lat1 = from->lat_deg * DEG_TO_RAD;
    const double lat2 = to->lat_deg * DEG_TO_RAD;
    const double dlon = (to->lon_deg - from->lon_deg) * DEG_TO_RAD;

    const double y = sin(dlon) * cos(lat2);
    const double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon);
    double bearing = atan2(y, x) * RAD_TO_DEG;

    while (bearing < 0.0) {
        bearing += 360.0;
    }

    while (bearing >= 360.0) {
        bearing -= 360.0;
    }

    return (float)bearing;
}
