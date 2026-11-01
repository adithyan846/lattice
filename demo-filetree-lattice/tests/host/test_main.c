#include "test_assert.h"

int g_test_failures = 0;

void test_vec3(void);
void test_quat(void);
void test_pid(void);
void test_geo(void);
void test_mission(void);
void test_atmosphere(void);
void test_rtl(void);
void test_attitude_controller(void);
void test_rate_controller(void);
void test_position_controller(void);
void test_mixer(void);
void test_actuator_limits(void);
void test_log_format(void);
void test_mavlink_framing(void);
void test_mavlink_command_mapping(void);
void test_mavlink_mission(void);
void test_waypoint(void);

int main(void)
{
    printf("Freertrtos host unit tests\n");
    printf("==========================\n");

    RUN_TEST(test_vec3);
    RUN_TEST(test_quat);
    RUN_TEST(test_pid);
    RUN_TEST(test_geo);
    RUN_TEST(test_mission);
    RUN_TEST(test_atmosphere);
    RUN_TEST(test_rtl);
    RUN_TEST(test_attitude_controller);
    RUN_TEST(test_rate_controller);
    RUN_TEST(test_position_controller);
    RUN_TEST(test_mixer);
    RUN_TEST(test_actuator_limits);
    RUN_TEST(test_log_format);
    RUN_TEST(test_mavlink_framing);
    RUN_TEST(test_mavlink_command_mapping);
    RUN_TEST(test_mavlink_mission);
    RUN_TEST(test_waypoint);

    printf("==========================\n");

    if (g_test_failures != 0) {
        printf("%d assertion(s) failed\n", g_test_failures);
        return 1;
    }

    printf("all tests passed\n");
    return 0;
}
