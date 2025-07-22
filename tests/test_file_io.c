#include "test_framework.h"
#include "../include/ciary.h"
#include <unistd.h>
#include <sys/stat.h>

static char* test_journal_dir = NULL;
static config_t test_config;

void setup_file_io_test() {
    test_journal_dir = create_temp_dir();
    if (test_journal_dir) {
        load_default_config(&test_config);
        strncpy(test_config.journal_directory, test_journal_dir, sizeof(test_config.journal_directory) - 1);
        test_config.journal_directory[sizeof(test_config.journal_directory) - 1] = '\0';
        
        // Create journal directory
        mkdir(test_journal_dir, 0755);
    }
}

void cleanup_file_io_test() {
    if (test_journal_dir) {
        remove_temp_dir(test_journal_dir);
        test_journal_dir = NULL;
    }
}

void test_entry_path_generation() {
    TEST_CASE("Entry Path Generation");
    setup_file_io_test();
    
    if (test_journal_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    date_t test_date = {2024, 1, 15};
    char path[512];
    
    char* result = get_entry_path(test_date, path, &test_config);
    ASSERT_TRUE(result, "get_entry_path should succeed");
    
    char expected_path[512];
    snprintf(expected_path, sizeof(expected_path), "%s/2024-01-15.md", test_journal_dir);
    ASSERT_STR_EQ(expected_path, path, "Entry path should be correctly formatted");
    
    cleanup_file_io_test();
}

void test_entry_existence_check() {
    TEST_CASE("Entry Existence Check");
    setup_file_io_test();
    
    if (test_journal_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    date_t test_date = {2024, 1, 15};
    
    // Test non-existent entry
    ASSERT_FALSE(entry_exists(test_date, &test_config), "Entry should not exist initially");
    
    // Create an entry file
    char path[512];
    get_entry_path(test_date, path, &test_config);
    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file, "# 2024-01-15\n\n## 10:30:00\n\nTest entry\n");
        fclose(file);
        
        ASSERT_TRUE(entry_exists(test_date, &test_config), "Entry should exist after creation");
    }
    
    cleanup_file_io_test();
}

void test_entry_counting() {
    TEST_CASE("Entry Counting");
    setup_file_io_test();
    
    if (test_journal_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    date_t test_date = {2024, 1, 15};
    
    // Test empty entry count
    ASSERT_EQ(0, count_entries(test_date, &test_config), "Should have 0 entries initially");
    
    // Create an entry file with multiple time sections
    char path[512];
    get_entry_path(test_date, path, &test_config);
    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file, "# 2024-01-15\n\n");
        fprintf(file, "## 09:30:00\n\nMorning entry\n\n");
        fprintf(file, "## 14:22:15\n\nAfternoon entry\n\n");
        fprintf(file, "## 21:45:30\n\nEvening entry\n");
        fclose(file);
        
        ASSERT_EQ(3, count_entries(test_date, &test_config), "Should count 3 time-based entries");
    }
    
    cleanup_file_io_test();
}

void test_journal_directory_creation() {
    TEST_CASE("Journal Directory Creation");
    
    char* temp_dir = create_temp_dir();
    if (temp_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    config_t config;
    load_default_config(&config);
    
    // Set journal directory to non-existent path
    char journal_path[512];
    snprintf(journal_path, sizeof(journal_path), "%s/new_journal", temp_dir);
    strncpy(config.journal_directory, journal_path, sizeof(config.journal_directory) - 1);
    config.journal_directory[sizeof(config.journal_directory) - 1] = '\0';
    
    ASSERT_FALSE(access(journal_path, F_OK) == 0, "Journal directory should not exist initially");
    
    int result = ensure_journal_dir(&config);
    ASSERT_EQ(0, result, "ensure_journal_dir should succeed");
    ASSERT_TRUE(access(journal_path, F_OK) == 0, "Journal directory should exist after ensure_journal_dir");
    
    // Verify it's actually a directory
    struct stat st;
    if (stat(journal_path, &st) == 0) {
        ASSERT_TRUE(S_ISDIR(st.st_mode), "Created path should be a directory");
    }
    
    remove_temp_dir(temp_dir);
}

void test_file_format_validation() {
    TEST_CASE("Entry File Format Validation");
    setup_file_io_test();
    
    if (test_journal_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    date_t test_date = {2024, 2, 29}; // Leap year date
    char path[512];
    get_entry_path(test_date, path, &test_config);
    
    // Create entry with proper format
    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file, "# 2024-02-29\n\n");
        fprintf(file, "## 10:30:45\n\n");
        fprintf(file, "This is a test entry with proper formatting.\n");
        fprintf(file, "Multiple lines are supported.\n\n");
        fprintf(file, "## 15:22:10\n\n");
        fprintf(file, "Second entry of the day.\n");
        fclose(file);
        
        ASSERT_TRUE(entry_exists(test_date, &test_config), "Properly formatted entry should exist");
        ASSERT_EQ(2, count_entries(test_date, &test_config), "Should correctly count formatted entries");
        
        // Test reading the file back
        file = fopen(path, "r");
        if (file) {
            char first_line[256];
            if (fgets(first_line, sizeof(first_line), file)) {
                first_line[strcspn(first_line, "\n")] = '\0';
                ASSERT_STR_EQ("# 2024-02-29", first_line, "First line should be date header");
            }
            fclose(file);
        }
    }
    
    cleanup_file_io_test();
}

void test_editor_detection() {
    TEST_CASE("Editor Detection");
    
    config_t config;
    load_default_config(&config);
    
    // Test auto detection
    strcpy(config.editor_preference, "auto");
    const char* detected_editor = get_actual_editor(&config);
    ASSERT_NOT_NULL(detected_editor, "Should detect an available editor");
    ASSERT_TRUE(strlen(detected_editor) > 0, "Detected editor name should not be empty");
    
    // Test specific editor preference
    strcpy(config.editor_preference, "nano");
    const char* nano_editor = get_actual_editor(&config);
    // Note: This test might fail if nano is not installed, which is acceptable
    ASSERT_NOT_NULL(nano_editor, "Should return an editor even if preferred is not available");
    
    // Test fallback
    strcpy(config.editor_preference, "nonexistent_editor_12345");
    const char* fallback_editor = get_actual_editor(&config);
    ASSERT_NOT_NULL(fallback_editor, "Should provide fallback editor");
    // Fallback could be any available editor (vi, nano, vim, etc.)
    ASSERT_TRUE(strlen(fallback_editor) > 0, "Fallback editor name should not be empty");
}

void test_path_expansion() {
    TEST_CASE("Path Expansion");
    
    // Test that paths are handled correctly
    config_t config;
    load_default_config(&config);
    
    // Verify that the default journal directory is set and reasonable
    ASSERT_TRUE(strlen(config.journal_directory) > 0, "Journal directory should be set");
    ASSERT_TRUE(config.journal_directory[0] == '/' || config.journal_directory[0] == '~', 
                "Journal directory should be absolute or home-relative path");
    
    // Test that config directory exists or can be created
    char config_dir[512];
    const char* home = getenv("HOME");
    if (home) {
        snprintf(config_dir, sizeof(config_dir), "%s/.config", home);
        
        struct stat st;
        if (stat(config_dir, &st) == 0) {
            ASSERT_TRUE(S_ISDIR(st.st_mode), "Config directory should be a directory if it exists");
        }
    }
}

void run_file_io_tests() {
    TEST_SUITE("File I/O Operations");
    
    test_entry_path_generation();
    test_entry_existence_check();
    test_entry_counting();
    test_journal_directory_creation();
    test_file_format_validation();
    test_editor_detection();
    test_path_expansion();
}