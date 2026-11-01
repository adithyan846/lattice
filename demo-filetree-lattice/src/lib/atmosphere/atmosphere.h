#ifndef LIB_ATMOSPHERE_ATMOSPHERE_H
#define LIB_ATMOSPHERE_ATMOSPHERE_H

#ifdef __cplusplus
extern "C" {
#endif

#define ATMOSPHERE_SEA_LEVEL_PRESSURE_PA 101325.0f

/** International Standard Atmosphere altitude from static pressure. */
float atmosphere_altitude_from_pressure(float pressure_pa, float sea_level_pa);

/** Inverse of atmosphere_altitude_from_pressure(). */
float atmosphere_pressure_from_altitude(float altitude_m, float sea_level_pa);

/** Simple density estimate at altitude, kg/m^3 (ISA sea-level density 1.225). */
float atmosphere_density(float altitude_m);

#ifdef __cplusplus
}
#endif

#endif /* LIB_ATMOSPHERE_ATMOSPHERE_H */
