#include <lib/atmosphere/atmosphere.h>

#include <math.h>

#define ISA_LAPSE_RATE 0.0065f
#define ISA_SEA_LEVEL_TEMP_K 288.15f
#define ISA_EXPONENT 5.255876f

float atmosphere_altitude_from_pressure(float pressure_pa, float sea_level_pa)
{
    if (pressure_pa <= 0.0f || sea_level_pa <= 0.0f) {
        return 0.0f;
    }

    return 44330.0f * (1.0f - powf(pressure_pa / sea_level_pa, 1.0f / ISA_EXPONENT));
}

float atmosphere_pressure_from_altitude(float altitude_m, float sea_level_pa)
{
    if (sea_level_pa <= 0.0f || altitude_m >= 44330.0f) {
        return 0.0f;
    }

    return sea_level_pa * powf(1.0f - altitude_m / 44330.0f, ISA_EXPONENT);
}

float atmosphere_density(float altitude_m)
{
    const float temperature_k = ISA_SEA_LEVEL_TEMP_K - ISA_LAPSE_RATE * altitude_m;

    if (temperature_k <= 0.0f) {
        return 0.0f;
    }

    const float pressure = atmosphere_pressure_from_altitude(altitude_m,
                                                            ATMOSPHERE_SEA_LEVEL_PRESSURE_PA);
    const float gas_constant = 287.05f;
    return pressure / (gas_constant * temperature_k);
}
