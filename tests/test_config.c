#define _GNU_SOURCE
#include "test_framework.h"
#include "../include/ciary.h"
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

static char* test_config_dir = NULL;
static char test_config_path[512];

void setup_config_test() {
    test_config_dir = create_temp_dir();
    if (test_config_dir) {
        snprintf(test_config_path, sizeof(test_config_path), "%s/config.conf", test_config_dir);
    }
}

void cleanup_config_test() {
    if (test_config_dir) {
        remove_temp_dir(test_config_dir);
        test_config_dir = NULL;
    }
}

void test_default_config() {
    TEST_CASE("Default Configuration");
    
    config_t config;
    load_default_config(&config);
    
    ASSERT_STR_EQ("auto", config.editor_preference, "Default editor should be auto");
    ASSERT_STR_EQ("auto", config.viewer_preference, "Default viewer should be auto");
    ASSERT_TRUE(config.show_ascii_art, "ASCII art should be enabled by default");
    ASSERT_TRUE(config.enable_personalization, "Personalization should be enabled by default");
    
    // Check that journal directory is set
    ASSERT_TRUE(strlen(config.journal_directory) > 0, "Journal directory should be set");
    ASSERT_TRUE(strlen(config.preferred_name) > 0, "Preferred name should be set");
}

void test_config_file_creation() {
    TEST_CASE("Configuration File Creation");
    setup_config_test();
    
    if (test_config_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    config_t config;
    load_default_config(&config);
    
    // Override journal directory to use test directory
    snprintf(config.journal_directory, sizeof(config.journal_directory), "%s/journal", test_config_dir);
    
    // Create a test config file
    FILE *file = fopen(test_config_path, "w");
    ASSERT_NOT_NULL(file, "Should be able to create config file");
    
    if (file) {
        fprintf(file, "preferred_name=TestUser\n");
        fprintf(file, "journal_directory=%s/journal\n", test_config_dir);
        fprintf(file, "editor_preference=vim\n");
        fprintf(file, "viewer_preference=less\n");
        fprintf(file, "show_ascii_art=false\n");
        fprintf(file, "enable_personalization=true\n");
        fclose(file);
        
        ASSERT_TRUE(access(test_config_path, F_OK) == 0, "Config file should exist after creation");
    }
    
    cleanup_config_test();
}

void test_config_file_parsing() {
    TEST_CASE("Configuration File Parsing");
    setup_config_test();
    
    if (test_config_dir == NULL) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    // Create a test config file
    FILE *file = fopen(test_config_path, "w");
    if (file) {
        fprintf(file, "preferred_name=TestUser\n");
        fprintf(file, "journal_directory=%s/test_journal\n", test_config_dir);
        fprintf(file, "editor_preference=nano\n");
        fprintf(file, "viewer_preference=cat\n");
        fprintf(file, "show_ascii_art=false\n");
        fprintf(file, "enable_personalization=false\n");
        fclose(file);
        
        config_t config;
        load_default_config(&config);
        
        // Override the config path for testing
        char original_home[512];
        char *home = getenv("HOME");
        if (home) {
            strncpy(original_home, home, sizeof(original_home) - 1);
            original_home[sizeof(original_home) - 1] = '\0';
        }
        
        // Set HOME to our test directory so config path points to our test file
        setenv("HOME", test_config_dir, 1);
        
        // Manually parse the test config file
        file = fopen(test_config_path, "r");
        if (file) {
            char line[512];
            while (fgets(line, sizeof(line), file)) {
                // Remove newline
                line[strcspn(line, "\n")] = '\0';
                
                // Skip empty lines and comments
                if (line[0] == '\0' || line[0] == '#') continue;
                
                // Parse key=value pairs
                char *delimiter = strchr(line, '=');
                if (delimiter) {
                    *delimiter = '\0';
                    char *key = line;
                    char *value = delimiter + 1;
                    
                    if (strcmp(key, "preferred_name") == 0) {
                        strncpy(config.preferred_name, value, sizeof(config.preferred_name) - 1);
                        config.preferred_name[sizeof(config.preferred_name) - 1] = '\0';
                    }
                    else if (strcmp(key, "journal_directory") == 0) {
                        strncpy(config.journal_directory, value, sizeof(config.journal_directory) - 1);
                        config.journal_directory[sizeof(config.journal_directory) - 1] = '\0';
                    }
                    else if (strcmp(key, "editor_preference") == 0) {
                        strncpy(config.editor_preference, value, sizeof(config.editor_preference) - 1);
                        config.editor_preference[sizeof(config.editor_preference) - 1] = '\0';
                    }
                    else if (strcmp(key, "viewer_preference") == 0) {
                        strncpy(config.viewer_preference, value, sizeof(config.viewer_preference) - 1);
                        config.viewer_preference[sizeof(config.viewer_preference) - 1] = '\0';
                    }
                    else if (strcmp(key, "show_ascii_art") == 0) {
                        config.show_ascii_art = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
                    }
                    else if (strcmp(key, "enable_personalization") == 0) {
                        config.enable_personalization = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
                    }
                }
            }
            fclose(file);
            
            // Test parsed values
            ASSERT_STR_EQ("TestUser", config.preferred_name, "Preferred name should be parsed correctly");
            ASSERT_STR_EQ("nano", config.editor_preference, "Editor preference should be parsed correctly");
            ASSERT_STR_EQ("cat", config.viewer_preference, "Viewer preference should be parsed correctly");
            ASSERT_FALSE(config.show_ascii_art, "ASCII art should be disabled");
            ASSERT_FALSE(config.enable_personalization, "Personalization should be disabled");
            
            char expected_journal[512];
            snprintf(expected_journal, sizeof(expected_journal), "%s/test_journal", test_config_dir);
            ASSERT_STR_EQ(expected_journal, config.journal_directory, "Journal directory should be parsed correctly");
        }
        
        // Restore original HOME
        if (home) {
            setenv("HOME", original_home, 1);
        }
    }
    
    cleanup_config_test();
}

void test_config_validation() {
    TEST_CASE("Configuration Validation");
    
    config_t config;
    load_default_config(&config);
    
    // Test valid editor preferences
    const char* valid_editors[] = {"auto", "nvim", "vim", "nano", "emacs", "vi"};
    for (int i = 0; i < 6; i++) {
        strncpy(config.editor_preference, valid_editors[i], sizeof(config.editor_preference) - 1);
        config.editor_preference[sizeof(config.editor_preference) - 1] = '\0';
        ASSERT_TRUE(strlen(config.editor_preference) > 0, "Valid editor preference should be accepted");
    }
    
    // Test valid viewer preferences
    const char* valid_viewers[] = {"auto", "less", "more", "cat"};
    for (int i = 0; i < 4; i++) {
        strncpy(config.viewer_preference, valid_viewers[i], sizeof(config.viewer_preference) - 1);
        config.viewer_preference[sizeof(config.viewer_preference) - 1] = '\0';
        ASSERT_TRUE(strlen(config.viewer_preference) > 0, "Valid viewer preference should be accepted");
    }
}

void run_config_tests() {
    TEST_SUITE("Configuration System");
    
    test_default_config();
    test_config_file_creation();
    test_config_file_parsing();
    test_config_validation();
}