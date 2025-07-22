#include "test_framework.h"
#include "../include/ciary.h"
#include <unistd.h>
#include <sys/stat.h>

static char* integration_test_dir = NULL;

void setup_integration_test() {
    integration_test_dir = create_temp_dir();
}

void cleanup_integration_test() {
    if (integration_test_dir) {
        remove_temp_dir(integration_test_dir);
        integration_test_dir = NULL;
    }
}

void test_full_workflow() {
    TEST_CASE("Full Application Workflow");
    setup_integration_test();
    
    if (integration_test_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    // 1. Initialize configuration
    config_t config;
    load_default_config(&config);
    
    // Set up test environment
    char journal_dir[512];
    snprintf(journal_dir, sizeof(journal_dir), "%s/journal", integration_test_dir);
    strncpy(config.journal_directory, journal_dir, sizeof(config.journal_directory) - 1);
    config.journal_directory[sizeof(config.journal_directory) - 1] = '\0';
    
    strcpy(config.preferred_name, "TestUser");
    strcpy(config.editor_preference, "auto");
    config.show_ascii_art = false;
    config.enable_personalization = true;
    
    // 2. Ensure journal directory is created
    int dir_result = ensure_journal_dir(&config);
    ASSERT_EQ(0, dir_result, "Should successfully create journal directory");
    ASSERT_TRUE(access(journal_dir, F_OK) == 0, "Journal directory should exist");
    
    // 3. Test entry creation for multiple dates
    date_t dates[] = {
        {2024, 1, 1},   // New Year
        {2024, 2, 29},  // Leap year day
        {2024, 12, 31}  // End of year
    };
    
    for (int i = 0; i < 3; i++) {
        date_t date = dates[i];
        
        // Check entry doesn't exist initially
        ASSERT_FALSE(entry_exists(date, &config), "Entry should not exist initially");
        
        // Create entry file manually (simulating editor operation)
        char path[512];
        get_entry_path(date, path, &config);
        
        FILE *file = fopen(path, "w");
        ASSERT_NOT_NULL(file, "Should be able to create entry file");
        
        if (file) {
            fprintf(file, "# %04d-%02d-%02d\n\n", date.year, date.month, date.day);
            fprintf(file, "## 09:30:00\n\nMorning entry for day %d\n\n", i + 1);
            fprintf(file, "## 18:45:00\n\nEvening reflection for day %d\n", i + 1);
            fclose(file);
            
            // Verify entry exists and has correct count
            ASSERT_TRUE(entry_exists(date, &config), "Entry should exist after creation");
            ASSERT_EQ(2, count_entries(date, &config), "Should have 2 time entries");
        }
    }
    
    // 4. Test calendar navigation scenarios
    date_t current_date = get_current_date();
    ASSERT_TRUE(is_today(current_date), "Current date should be today");
    
    // Test date arithmetic
    int days_jan = days_in_month(1, 2024);
    ASSERT_EQ(31, days_jan, "January should have 31 days");
    
    int days_feb = days_in_month(2, 2024);
    ASSERT_EQ(29, days_feb, "February 2024 should have 29 days (leap year)");
    
    // 5. Test configuration persistence (simulate)
    char config_path[512];
    snprintf(config_path, sizeof(config_path), "%s/config.conf", integration_test_dir);
    
    FILE *config_file = fopen(config_path, "w");
    if (config_file) {
        fprintf(config_file, "preferred_name=%s\n", config.preferred_name);
        fprintf(config_file, "journal_directory=%s\n", config.journal_directory);
        fprintf(config_file, "editor_preference=%s\n", config.editor_preference);
        fprintf(config_file, "show_ascii_art=%s\n", config.show_ascii_art ? "true" : "false");
        fprintf(config_file, "enable_personalization=%s\n", config.enable_personalization ? "true" : "false");
        fclose(config_file);
        
        ASSERT_TRUE(access(config_path, F_OK) == 0, "Config file should be created");
    }
    
    cleanup_integration_test();
}

void test_concurrent_access() {
    TEST_CASE("Concurrent Access Handling");
    setup_integration_test();
    
    if (integration_test_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    config_t config;
    load_default_config(&config);
    
    char journal_dir[512];
    snprintf(journal_dir, sizeof(journal_dir), "%s/journal", integration_test_dir);
    strncpy(config.journal_directory, journal_dir, sizeof(config.journal_directory) - 1);
    config.journal_directory[sizeof(config.journal_directory) - 1] = '\0';
    
    ensure_journal_dir(&config);
    
    date_t test_date = {2024, 6, 15};
    
    // Simulate concurrent access by creating multiple entries rapidly
    for (int i = 0; i < 5; i++) {
        char path[512];
        get_entry_path(test_date, path, &config);
        
        FILE *file = fopen(path, "a");  // Append mode to simulate concurrent writes
        if (file) {
            fprintf(file, "## %02d:00:00\n\nEntry %d\n\n", 10 + i, i);
            fclose(file);
        }
    }
    
    // Verify all entries were written
    int final_count = count_entries(test_date, &config);
    ASSERT_EQ(5, final_count, "Should have 5 entries after concurrent writes");
    
    cleanup_integration_test();
}

void test_edge_cases() {
    TEST_CASE("Edge Cases and Error Handling");
    setup_integration_test();
    
    if (integration_test_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    config_t config;
    load_default_config(&config);
    
    // Test with invalid journal directory
    strcpy(config.journal_directory, "/root/invalid_permission_dir");
    int result = ensure_journal_dir(&config);
    // This might succeed or fail depending on system, but shouldn't crash
    printf("  Note: ensure_journal_dir with invalid path returned %d\n", result);
    
    // Test leap year edge cases
    ASSERT_TRUE(is_leap_year(2000), "2000 should be leap year");
    ASSERT_FALSE(is_leap_year(1900), "1900 should not be leap year");
    ASSERT_TRUE(is_leap_year(2400), "2400 should be leap year");
    
    // Test month boundary conditions
    date_t dec31 = {2023, 12, 31};
    date_t jan1 = {2024, 1, 1};
    ASSERT_TRUE(date_compare(dec31, jan1) < 0, "Dec 31, 2023 < Jan 1, 2024");
    
    // Test February 29 in leap vs non-leap years
    ASSERT_EQ(29, days_in_month(2, 2024), "Feb 2024 has 29 days");
    ASSERT_EQ(28, days_in_month(2, 2023), "Feb 2023 has 28 days");
    
    // Test day of week for known dates
    int dow_new_year_2024 = day_of_week(2024, 1, 1);
    ASSERT_EQ(1, dow_new_year_2024, "Jan 1, 2024 is Monday (1)");
    
    cleanup_integration_test();
}

void test_data_integrity() {
    TEST_CASE("Data Integrity and Persistence");
    setup_integration_test();
    
    if (integration_test_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    config_t config;
    load_default_config(&config);
    
    char journal_dir[512];
    snprintf(journal_dir, sizeof(journal_dir), "%s/journal", integration_test_dir);
    strncpy(config.journal_directory, journal_dir, sizeof(config.journal_directory) - 1);
    config.journal_directory[sizeof(config.journal_directory) - 1] = '\0';
    
    ensure_journal_dir(&config);
    
    date_t test_date = {2024, 7, 4}; // Independence Day
    
    // Create entry with special characters and unicode
    char path[512];
    get_entry_path(test_date, path, &config);
    
    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file, "# 2024-07-04\n\n");
        fprintf(file, "## 10:30:00\n\n");
        fprintf(file, "Entry with special chars: !@#$%%^&*()_+-=[]{}|;':\",./<>?\n");
        fprintf(file, "Unicode test: 🇺🇸 🎆 🎇 ★ ☆\n");
        fprintf(file, "Markdown test: **bold** *italic* `code`\n");
        fprintf(file, "Multi-line\nentry with\nvarious formatting\n\n");
        fprintf(file, "## 20:15:30\n\n");
        fprintf(file, "Evening entry with [link](https://example.com)\n");
        fclose(file);
        
        ASSERT_TRUE(entry_exists(test_date, &config), "Entry with special content should exist");
        ASSERT_EQ(2, count_entries(test_date, &config), "Should correctly count entries with special content");
        
        // Verify content integrity by reading back
        file = fopen(path, "r");
        if (file) {
            char buffer[1024];
            size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file);
            buffer[bytes_read] = '\0';
            fclose(file);
            
            ASSERT_TRUE(strstr(buffer, "special chars") != NULL, "Special characters should be preserved");
            ASSERT_TRUE(strstr(buffer, "🇺🇸") != NULL, "Unicode should be preserved");
            ASSERT_TRUE(strstr(buffer, "**bold**") != NULL, "Markdown formatting should be preserved");
        }
    }
    
    cleanup_integration_test();
}

void run_integration_tests() {
    TEST_SUITE("Integration Tests");
    
    test_full_workflow();
    test_concurrent_access();
    test_edge_cases();
    test_data_integrity();
}