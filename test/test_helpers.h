#ifndef INTENT_TEST_HELPERS_H
#define INTENT_TEST_HELPERS_H

#include <stdio.h>

extern int gTestPassed;
extern int gTestFailed;

#define TEST_ASSERT(cond) do { \
    if (cond) { \
        gTestPassed++; \
    } else { \
        gTestFailed++; \
        printf("  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define TEST_SUMMARY() do { \
    printf("\n=== Test Summary ===\n"); \
    printf("Passed: %d\n", gTestPassed); \
    printf("Failed: %d\n", gTestFailed); \
    if (gTestFailed > 0) return 1; \
    return 0; \
} while (0)

#endif
