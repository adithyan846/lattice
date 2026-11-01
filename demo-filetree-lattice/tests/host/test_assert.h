#ifndef TESTS_HOST_TEST_ASSERT_H
#define TESTS_HOST_TEST_ASSERT_H

#include <math.h>
#include <stdio.h>

extern int g_test_failures;

#define TEST_ASSERT(condition)                                                       \
    do {                                                                             \
        if (!(condition)) {                                                          \
            fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);   \
            g_test_failures++;                                                       \
        }                                                                            \
    } while (0)

#define TEST_ASSERT_NEAR(actual, expected, tolerance)                                \
    do {                                                                             \
        const double _a = (double)(actual);                                          \
        const double _e = (double)(expected);                                        \
        const double _t = (double)(tolerance);                                       \
        if (fabs(_a - _e) > _t) {                                                    \
            fprintf(stderr,                                                          \
                    "  FAIL %s:%d: %s = %.9f, expected %.9f +/- %.9f\n",             \
                    __FILE__, __LINE__, #actual, _a, _e, _t);                        \
            g_test_failures++;                                                       \
        }                                                                            \
    } while (0)

#define RUN_TEST(fn)                                                                 \
    do {                                                                             \
        const int before = g_test_failures;                                          \
        printf("running %s ...\n", #fn);                                             \
        fn();                                                                        \
        printf("  %s\n", (g_test_failures == before) ? "PASS" : "FAILED");           \
    } while (0)

#endif /* TESTS_HOST_TEST_ASSERT_H */
