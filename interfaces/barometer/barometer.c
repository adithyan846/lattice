#include "barometer.h"
#include "bmp280.h"
#include "board.h"
#include <stddef.h>

/*
 * BAROMETER INTERFACE
 *
 * Device selection and update dispatch:
 *     Currently BMP280 on BOARD_SPI_BARO_BUS
 *
 * Data handling:
 *     Receives measurements from the device driver
 *     Derives altitude from pressure
 *     Stores the latest measurement
 *     Provides data to application/core consumers
 */

static barometer_data_t latest_data;
static bool data_available = false;

int barometer_init(void)
{
    return bmp280_init(BOARD_SPI_BARO_BUS);
}

int barometer_update(void)
{
    return bmp280_update();
}

void barometer_publish(const barometer_data_t *data)
{
    if (data == NULL)
      return;

    latest_data = *data;
    latest_data.altitude_cm = barometer_pressure_to_altitude_cm(data->pressure_pa);
    latest_data.valid = true;
    data_available = true;
}

bool barometer_get_latest(barometer_data_t *data)
{
    if (data == NULL || !data_available)
      return false;

    *data = latest_data;
    return true;
}

/*
 * Altitude in centimetres, using 32-bit integer maths only.
 *
 * powf() returns 1.0 on this toolchain's nano-libm and its 64-bit division
 * helper misbehaves on THEJAS32, so the barometric formula
 *
 *     h = 44307.69396 * (1 - (P / 101325) ^ 0.190284)
 *
 * is pre-computed for 30000..110000 Pa in 250 Pa steps and linearly
 * interpolated.  Every intermediate fits in 32 bits (at most ~1.3e6).
 * Worst-case interpolation error is about 6 cm over the full range.
 */
#define BARO_ALT_BASE_PA   (30000U)
#define BARO_ALT_STEP_PA   (250U)

static const int32_t altitude_cm_table[] = {
        916007,  910452,  904935,  899454,  894009,  888599,  883224,  877884,
        872577,  867304,  862064,  856857,  851682,  846538,  841425,  836344,
        831293,  826272,  821280,  816317,  811384,  806478,  801602,  796752,
        791930,  787135,  782367,  777625,  772909,  768219,  763554,  758915,
        754300,  749710,  745144,  740602,  736083,  731588,  727117,  722667,
        718241,  713837,  709455,  705095,  700756,  696439,  692143,  687868,
        683613,  679379,  675166,  670972,  666798,  662644,  658509,  654393,
        650297,  646218,  642159,  638118,  634095,  630091,  626104,  622135,
        618183,  614250,  610332,  606432,  602549,  598683,  594833,  591000,
        587182,  583381,  579596,  575826,  572072,  568334,  564611,  560903,
        557210,  553532,  549869,  546220,  542586,  538967,  535361,  531770,
        528193,  524630,  521080,  517544,  514022,  510514,  507018,  503536,
        500067,  496611,  493167,  489736,  486318,  482913,  479520,  476139,
        472771,  469415,  466071,  462739,  459418,  456110,  452813,  449527,
        446254,  442991,  439740,  436501,  433272,  430055,  426848,  423652,
        420467,  417293,  414129,  410976,  407834,  404702,  401580,  398468,
        395367,  392276,  389195,  386123,  383062,  380010,  376969,  373937,
        370914,  367901,  364898,  361904,  358919,  355944,  352977,  350020,
        347073,  344134,  341204,  338282,  335371,  332467,  329572,  326686,
        323809,  320940,  318080,  315228,  312384,  309549,  306722,  303903,
        301093,  298291,  295496,  292710,  289931,  287161,  284399,  281643,
        278897,  276157,  273425,  270702,  267985,  265276,  262575,  259881,
        257194,  254514,  251842,  249178,  246520,  243870,  241226,  238590,
        235961,  233339,  230724,  228115,  225514,  222919,  220332,  217750,
        215176,  212608,  210047,  207493,  204945,  202403,  199868,  197340,
        194818,  192302,  189793,  187290,  184793,  182303,  179818,  177340,
        174868,  172402,  169942,  167488,  165041,  162599,  160163,  157733,
        155308,  152890,  150478,  148071,  145670,  143274,  140885,  138501,
        136122,  133750,  131382,  129021,  126664,  124314,  121968,  119628,
        117294,  114965,  112641,  110323,  108010,  105702,  103399,  101101,
         98809,   96522,   94240,   91963,   89691,   87424,   85163,   82906,
         80654,   78407,   76165,   73928,   71696,   69469,   67246,   65028,
         62815,   60607,   58404,   56205,   54011,   51822,   49637,   47457,
         45282,   43111,   40945,   38783,   36626,   34473,   32325,   30181,
         28041,   25907,   23776,   21650,   19528,   17410,   15297,   13188,
         11084,    8983,    6887,    4795,    2707,     624,   -1455,   -3530,
         -5601,   -7668,   -9731,  -11790,  -13845,  -15895,  -17942,  -19985,
        -22024,  -24058,  -26089,  -28116,  -30139,  -32158,  -34173,  -36185,
        -38192,  -40196,  -42196,  -44192,  -46184,  -48173,  -50157,  -52139,
        -54116,  -56089,  -58059,  -60025,  -61988,  -63947,  -65902,  -67854,
        -69802,
};

int32_t barometer_pressure_to_altitude_cm(uint32_t pressure_pa)
{
    const uint32_t entry_count =
      (uint32_t)(sizeof(altitude_cm_table) / sizeof(altitude_cm_table[0]));
    uint32_t offset;
    uint32_t index;
    uint32_t remainder;
    int32_t delta;

    /* Clamp to the table range (30000..110000 Pa). */
    if (pressure_pa <= BARO_ALT_BASE_PA)
      return altitude_cm_table[0];

    if (pressure_pa >= BARO_ALT_BASE_PA + (entry_count - 1U) * BARO_ALT_STEP_PA)
      return altitude_cm_table[entry_count - 1U];

    offset = pressure_pa - BARO_ALT_BASE_PA;
    index = offset / BARO_ALT_STEP_PA;
    remainder = offset % BARO_ALT_STEP_PA;

    delta = altitude_cm_table[index + 1U] - altitude_cm_table[index];

    return altitude_cm_table[index] +
           (delta * (int32_t)remainder) / (int32_t)BARO_ALT_STEP_PA;
}
