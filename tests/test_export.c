#include "test_framework.h"
#include "../include/ciary.h"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// Use external test counters from framework
extern int test_count;
extern int test_passed; 
extern int test_failed;

// Test helper functions
static char* create_test_journal_dir(void) {
    static char temp_dir[256];
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/ciary_test_%d", getpid());
    if (mkdir(temp_dir, 0755) != 0 && errno != EEXIST) {
        // If mkdir fails and directory doesn't exist, return NULL
        return NULL;
    }
    return temp_dir;
}

static void cleanup_test_journal_dir(const char* dir) {
    char command[512];
    snprintf(command, sizeof(command), "rm -rf %s", dir);
    system(command);
}

static void create_test_entry(const char* journal_dir, const char* date, const char* content) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s.md", journal_dir, date);
    
    FILE* file = fopen(filepath, "w");
    if (file) {
        fprintf(file, "# %s\n\n%s\n", date, content);
        fclose(file);
    }
}

// Test date comparison function
void test_date_comparison(void) {
    TEST_CASE("Date Comparison Functions");
    
    // Test equal dates
    date_t date1 = {2024, 7, 15};
    date_t date2 = {2024, 7, 15};
    ASSERT_EQ(0, date_compare(date1, date2), "Equal dates should return 0");
    
    // Test date1 < date2
    date_t date3 = {2024, 7, 14};
    date_t date4 = {2024, 7, 15};
    ASSERT_EQ(-1, date_compare(date3, date4), "Earlier date should return -1");
    
    // Test date1 > date2
    date_t date5 = {2024, 7, 16};
    date_t date6 = {2024, 7, 15};
    ASSERT_EQ(1, date_compare(date5, date6), "Later date should return 1");
    
    // Test different months
    date_t date7 = {2024, 6, 15};
    date_t date8 = {2024, 7, 15};
    ASSERT_EQ(-1, date_compare(date7, date8), "Earlier month should return -1");
    
    // Test different years
    date_t date9 = {2023, 7, 15};
    date_t date10 = {2024, 7, 15};
    ASSERT_EQ(-1, date_compare(date9, date10), "Earlier year should return -1");
    
    // Test cross-year comparison
    date_t date11 = {2024, 12, 31};
    date_t date12 = {2025, 1, 1};
    ASSERT_EQ(-1, date_compare(date11, date12), "Dec 31 should be earlier than Jan 1 next year");
}

// Test date parsing functions
void test_date_parsing(void) {
    TEST_CASE("Date Parsing Functions");
    
    date_t parsed_date;
    
    // Test valid date parsing
    bool result = parse_date_from_filename("2024-07-15.md", &parsed_date);
    ASSERT_TRUE(result, "Should successfully parse valid date filename");
    ASSERT_EQ(2024, parsed_date.year, "Year should be parsed correctly");
    ASSERT_EQ(7, parsed_date.month, "Month should be parsed correctly");
    ASSERT_EQ(15, parsed_date.day, "Day should be parsed correctly");
    
    // Test invalid filename
    result = parse_date_from_filename("invalid.md", &parsed_date);
    ASSERT_FALSE(result, "Should fail to parse invalid filename");
    
    // Test filename without extension
    result = parse_date_from_filename("2024-07-15", &parsed_date);
    ASSERT_FALSE(result, "Should fail to parse filename without .md extension");
    
    // Test malformed date
    result = parse_date_from_filename("2024-13-45.md", &parsed_date);
    ASSERT_FALSE(result, "Should fail to parse invalid date values");
}

// Test chronological sorting
void test_chronological_sorting(void) {
    TEST_CASE("Chronological Sorting");
    
    char* test_dir = create_test_journal_dir();
    if (!test_dir) {
        // If directory creation fails, skip this test
        return;
    }
    
    // Create test entries in random order
    create_test_entry(test_dir, "2024-07-25", "## 10:00:00\n\nNewest entry");
    create_test_entry(test_dir, "2024-07-20", "## 14:00:00\n\nOldest entry");
    create_test_entry(test_dir, "2024-07-23", "## 16:00:00\n\nMiddle entry");
    create_test_entry(test_dir, "2024-07-21", "## 12:00:00\n\nSecond entry");
    
    // Setup export options
    export_options_t options;
    options.start_date = (date_t){2024, 7, 20};
    options.end_date = (date_t){2024, 7, 25};
    options.include_empty_days = 0;
    strcpy(options.output_path, "/tmp");
    
    config_t config;
    strcpy(config.journal_directory, test_dir);
    
    // Test collection and sorting
    char **entry_files;
    int file_count;
    
    bool collection_result = collect_entries_in_range(&options, &config, &entry_files, &file_count);
    ASSERT_TRUE(collection_result, "Should successfully collect entries");
    ASSERT_EQ(4, file_count, "Should find all 4 test entries");
    
    if (collection_result && file_count == 4) {
        // Check chronological order
        char* filename1 = strrchr(entry_files[0], '/');
        char* filename2 = strrchr(entry_files[1], '/');
        char* filename3 = strrchr(entry_files[2], '/');
        char* filename4 = strrchr(entry_files[3], '/');
        
        if (filename1) filename1++; else filename1 = entry_files[0];
        if (filename2) filename2++; else filename2 = entry_files[1];
        if (filename3) filename3++; else filename3 = entry_files[2];
        if (filename4) filename4++; else filename4 = entry_files[3];
        
        ASSERT_STR_EQ("2024-07-20.md", filename1, "First file should be oldest date");
        ASSERT_STR_EQ("2024-07-21.md", filename2, "Second file should be second oldest");
        ASSERT_STR_EQ("2024-07-23.md", filename3, "Third file should be third oldest");
        ASSERT_STR_EQ("2024-07-25.md", filename4, "Fourth file should be newest date");
        
        // Cleanup allocated memory
        for (int i = 0; i < file_count; i++) {
            free(entry_files[i]);
        }
        free(entry_files);
    }
    
    cleanup_test_journal_dir(test_dir);
}

// Test export format validation
void test_export_format_validation(void) {
    TEST_CASE("Export Format Validation");
    
    // Test valid formats
    ASSERT_TRUE(EXPORT_FORMAT_HTML >= 0 && EXPORT_FORMAT_HTML <= 2, "HTML format should be valid");
    ASSERT_TRUE(EXPORT_FORMAT_PDF >= 0 && EXPORT_FORMAT_PDF <= 2, "PDF format should be valid");
    ASSERT_TRUE(EXPORT_FORMAT_MARKDOWN >= 0 && EXPORT_FORMAT_MARKDOWN <= 2, "Markdown format should be valid");
    
    // Test format names
    const char* format_names[] = {"HTML", "PDF", "Markdown"};
    for (int i = 0; i < 3; i++) {
        ASSERT_NOT_NULL(format_names[i], "Format name should not be null");
    }
}

// Test date range validation
void test_date_range_validation(void) {
    TEST_CASE("Date Range Validation");
    
    // Test valid date range
    date_t start = {2024, 7, 1};
    date_t end = {2024, 7, 31};
    ASSERT_TRUE(date_compare(start, end) <= 0, "Start date should be <= end date for valid range");
    
    // Test invalid date range (start > end)
    date_t invalid_start = {2024, 7, 31};
    date_t invalid_end = {2024, 7, 1};
    ASSERT_TRUE(date_compare(invalid_start, invalid_end) > 0, "Invalid range should have start > end");
    
    // Test same date range
    date_t same_start = {2024, 7, 15};
    date_t same_end = {2024, 7, 15};
    ASSERT_EQ(0, date_compare(same_start, same_end), "Same date range should be valid");
}

// Test markdown export functionality
void test_markdown_export(void) {
    TEST_CASE("Markdown Export Functionality");
    
    char* test_dir = create_test_journal_dir();
    if (!test_dir) {
        // If directory creation fails, skip this test
        return;
    }
    
    // Create a simple test entry
    create_test_entry(test_dir, "2024-07-15", "## 10:30:00\n\nTest entry for markdown export");
    
    // Setup export options
    export_options_t options;
    options.start_date = (date_t){2024, 7, 15};
    options.end_date = (date_t){2024, 7, 15};
    options.format = EXPORT_FORMAT_MARKDOWN;
    options.include_empty_days = 0;
    strcpy(options.output_path, "/tmp");
    
    config_t config;
    strcpy(config.journal_directory, test_dir);
    strcpy(config.preferred_name, "Test User");
    
    // Test export
    bool export_result = export_entries(&options, &config);
    ASSERT_TRUE(export_result, "Markdown export should succeed");
    
    // Check if export file was created
    char expected_file[512];
    snprintf(expected_file, sizeof(expected_file), "/tmp/ciary_export_2024-07-15_to_2024-07-15.md");
    
    FILE* export_file = fopen(expected_file, "r");
    ASSERT_NOT_NULL(export_file, "Export file should be created");
    
    if (export_file) {
        // Read and verify content
        char buffer[1024];
        bool found_title = false;
        bool found_entry = false;
        
        while (fgets(buffer, sizeof(buffer), export_file)) {
            if (strstr(buffer, "# Ciary Export")) {
                found_title = true;
            }
            if (strstr(buffer, "Test entry for markdown export")) {
                found_entry = true;
            }
        }
        
        ASSERT_TRUE(found_title, "Export should contain title");
        ASSERT_TRUE(found_entry, "Export should contain entry content");
        
        fclose(export_file);
        unlink(expected_file);
    }
    
    cleanup_test_journal_dir(test_dir);
}

// Main test runner for export functionality
void run_export_tests(void) {
    TEST_SUITE("Export Functionality Tests");
    
    test_date_comparison();
    test_date_parsing();
    test_chronological_sorting();
    test_export_format_validation();
    test_date_range_validation();
    test_markdown_export();
    
    TEST_SUMMARY();
}

// Standalone test runner for this module
#ifdef TEST_EXPORT_STANDALONE
int main(void) {
    setup_test_environment();
    run_export_tests();
    cleanup_test_environment();
    return (test_failed == 0) ? 0 : 1;
}
#endif