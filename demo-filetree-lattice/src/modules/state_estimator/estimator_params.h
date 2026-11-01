#ifndef MODULES_STATE_ESTIMATOR_ESTIMATOR_PARAMS_H
#define MODULES_STATE_ESTIMATOR_ESTIMATOR_PARAMS_H

/* Parameter names used by the estimator. Values live in param_table.c and
 * config/boards/thejas32/default.params. */

#define PARAM_EKF_ATT_ACC  "EKF_ATT_ACC"
#define PARAM_EKF_ATT_MAG  "EKF_ATT_MAG"
#define PARAM_EKF_POS_GPS  "EKF_POS_GPS"
#define PARAM_EKF_POS_BARO "EKF_POS_BARO"
#define PARAM_EKF_GPS_TIMEOUT_MS "EKF_GPS_TIMEOUT_MS"

#endif /* MODULES_STATE_ESTIMATOR_ESTIMATOR_PARAMS_H */
