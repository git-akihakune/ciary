#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for test suites
void run_utils_tests(void);
void run_config_tests(void);
void run_file_io_tests(void);
void run_export_tests(void);
void run_integration_tests(void);
void run_ui_tests(void);
void run_personalization_tests(void);

// Global test statistics
static int total_tests = 0;
static int total_passed = 0;
static int total_failed = 0;

void update_global_stats() {
    total_tests += test_count;
    total_passed += test_passed;
    total_failed += test_failed;
}

void print_final_summary() {
    printf("\n");
    printf("=====================================\n");
    printf("       FINAL TEST SUMMARY\n");
    printf("=====================================\n");
    printf("Total test cases: %d\n", total_tests);
    printf("Passed: %d\n", total_passed);
    printf("Failed: %d\n", total_failed);
    printf("Success rate: %.1f%%\n", total_tests > 0 ? (total_passed * 100.0 / total_tests) : 0.0);
    
    if (total_failed == 0) {
        printf("🎉 ALL TESTS PASSED! 🎉\n");
    } else {
        printf("❌ %d TEST(S) FAILED ❌\n", total_failed);
    }
    printf("=====================================\n");
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options] [test_suite]\n", program_name);
    printf("\nOptions:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --verbose  Enable verbose output\n");
    printf("  -q, --quiet    Suppress output except for failures\n");
    printf("\nTest Suites:\n");
    printf("  utils          Run utility function tests\n");
    printf("  config         Run configuration system tests\n");
    printf("  file_io        Run file I/O operation tests\n");
    printf("  export         Run export functionality tests\n");
    printf("  integration    Run integration tests\n");
    printf("  ui             Run UI/UX tests\n");
    printf("  personalization Run personalization system tests\n");
    printf("  all            Run all test suites (default)\n");
    printf("\nExamples:\n");
    printf("  %s                    # Run all tests\n", program_name);
    printf("  %s utils              # Run only utility tests\n", program_name);
    printf("  %s -v integration     # Run integration tests with verbose output\n", program_name);
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    bool quiet = false;
    const char* test_suite = "all";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        }
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        }
        else if (argv[i][0] != '-') {
            test_suite = argv[i];
        }
        else {
            printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Print header
    if (!quiet) {
        printf("=====================================\n");
        printf("         CIARY TEST SUITE\n");
        printf("=====================================\n");
        printf("Running test suite: %s\n", test_suite);
        if (verbose) printf("Verbose mode: ON\n");
        if (quiet) printf("Quiet mode: ON\n");
        printf("=====================================\n");
    }
    
    // Set up test environment
    setup_test_environment();
    
    // Run selected test suites
    if (strcmp(test_suite, "all") == 0) {
        // Run all test suites
        run_utils_tests();
        update_global_stats();
        
        run_config_tests();
        update_global_stats();
        
        run_file_io_tests();
        update_global_stats();
        
        run_export_tests();
        update_global_stats();
        
        run_integration_tests();
        update_global_stats();
        
        run_ui_tests();
        update_global_stats();
        
        run_personalization_tests();
        update_global_stats();
    }
    else if (strcmp(test_suite, "utils") == 0) {
        run_utils_tests();
        update_global_stats();
    }
    else if (strcmp(test_suite, "config") == 0) {
        run_config_tests();
        update_global_stats();
    }
    else if (strcmp(test_suite, "file_io") == 0) {
        run_file_io_tests();
        update_global_stats();
    }
    else if (strcmp(test_suite, "export") == 0) {
        run_export_tests();
        update_global_stats();
    }
    else if (strcmp(test_suite, "integration") == 0) {
        run_integration_tests();
        update_global_stats();
    }
    else if (strcmp(test_suite, "ui") == 0) {
        run_ui_tests();
        update_global_stats();
    }
    else if (strcmp(test_suite, "personalization") == 0) {
        run_personalization_tests();
        update_global_stats();
    }
    else {
        printf("Unknown test suite: %s\n", test_suite);
        print_usage(argv[0]);
        cleanup_test_environment();
        return 1;
    }
    
    // Clean up test environment
    cleanup_test_environment();
    
    // Print final summary
    if (!quiet) {
        print_final_summary();
    }
    
    // Return appropriate exit code
    return (total_failed == 0) ? 0 : 1;
}