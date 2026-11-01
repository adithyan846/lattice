#ifndef LIB_GEO_GEO_H
#define LIB_GEO_GEO_H

#include <lib/math/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif

/** WGS-84 mean Earth radius used for the local tangent-plane approximation. */
#define GEO_EARTH_RADIUS_M 6371000.0

typedef struct {
    double lat_deg;
    double lon_deg;
    float alt_m;
} geo_point_t;

typedef struct {
    double lat_deg;
    double lon_deg;
    float alt_m;
} geo_reference_t;

/**
 * Convert a global position to a local NED position relative to `ref`.
 *
 * This is an equirectangular (local tangent plane) approximation. It is
 * accurate to a few metres over a few kilometres and is intentionally simple;
 * use a proper geodesic implementation if you need long-baseline accuracy.
 */
void geo_global_to_local(const geo_point_t *global, const geo_reference_t *ref, vec3_t *local_ned);

/** Inverse of geo_global_to_local(). */
void geo_local_to_global(const vec3_t *local_ned, const geo_reference_t *ref, geo_point_t *global);

/** Great-circle distance in metres (haversine). */
float geo_distance_m(const geo_point_t *a, const geo_point_t *b);

/** Initial bearing from `from` to `to`, degrees clockwise from north [0, 360). */
float geo_bearing_deg(const geo_point_t *from, const geo_point_t *to);

#ifdef __cplusplus
}
#endif

#endif /* LIB_GEO_GEO_H */
