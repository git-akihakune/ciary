#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test framework macros and functions
extern int test_count;
extern int test_passed;
extern int test_failed;

#define TEST_SUITE(name) \
    printf("\n=== Test Suite: %s ===\n", name); \
    test_count = 0; \
    test_passed = 0; \
    test_failed = 0;

#define TEST_CASE(name) \
    printf("\n--- Test Case: %s ---\n", name);

#define ASSERT_TRUE(condition, message) \
    do { \
        test_count++; \
        if (condition) { \
            printf("✓ PASS: %s\n", message); \
            test_passed++; \
        } else { \
            printf("✗ FAIL: %s\n", message); \
            test_failed++; \
        } \
    } while(0)

#define ASSERT_FALSE(condition, message) \
    ASSERT_TRUE(!(condition), message)

#define ASSERT_EQ(expected, actual, message) \
    do { \
        test_count++; \
        if ((expected) == (actual)) { \
            printf("✓ PASS: %s (expected: %d, actual: %d)\n", message, (int)(expected), (int)(actual)); \
            test_passed++; \
        } else { \
            printf("✗ FAIL: %s (expected: %d, actual: %d)\n", message, (int)(expected), (int)(actual)); \
            test_failed++; \
        } \
    } while(0)

#define ASSERT_STR_EQ(expected, actual, message) \
    do { \
        test_count++; \
        if (strcmp((expected), (actual)) == 0) { \
            printf("✓ PASS: %s\n", message); \
            test_passed++; \
        } else { \
            printf("✗ FAIL: %s (expected: '%s', actual: '%s')\n", message, (expected), (actual)); \
            test_failed++; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr, message) \
    do { \
        test_count++; \
        if ((ptr) != NULL) { \
            printf("✓ PASS: %s\n", message); \
            test_passed++; \
        } else { \
            printf("✗ FAIL: %s (pointer is NULL)\n", message); \
            test_failed++; \
        } \
    } while(0)

#define ASSERT_NULL(ptr, message) \
    do { \
        test_count++; \
        if ((ptr) == NULL) { \
            printf("✓ PASS: %s\n", message); \
            test_passed++; \
        } else { \
            printf("✗ FAIL: %s (pointer is not NULL)\n", message); \
            test_failed++; \
        } \
    } while(0)

#define TEST_SUMMARY() \
    do { \
        printf("\n=== Test Summary ===\n"); \
        printf("Total tests: %d\n", test_count); \
        printf("Passed: %d\n", test_passed); \
        printf("Failed: %d\n", test_failed); \
        if (test_failed == 0) { \
            printf("🎉 All tests passed!\n"); \
        } else { \
            printf("❌ %d test(s) failed\n", test_failed); \
        } \
        printf("Success rate: %.1f%%\n", test_count > 0 ? (test_passed * 100.0 / test_count) : 0.0); \
    } while(0)

// Test utilities
void setup_test_environment(void);
void cleanup_test_environment(void);
char* create_temp_dir(void);
void remove_temp_dir(const char* path);

#endif // TEST_FRAMEWORK_H