#include "test_framework.h"
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

// Global test counters
int test_count = 0;
int test_passed = 0;
int test_failed = 0;

void setup_test_environment(void) {
    // Set up any global test environment
    printf("Setting up test environment...\n");
}

void cleanup_test_environment(void) {
    // Clean up global test environment
    printf("Cleaning up test environment...\n");
}

char* create_temp_dir(void) {
    static char temp_path[256];
    time_t now = time(NULL);
    snprintf(temp_path, sizeof(temp_path), "/tmp/ciary_test_%ld_%d", now, getpid());
    
    if (mkdir(temp_path, 0755) == 0) {
        return temp_path;
    }
    return NULL;
}

void remove_temp_dir(const char* path) {
    if (path == NULL) return;
    
    char command[512];
    snprintf(command, sizeof(command), "rm -rf \"%s\"", path);
    system(command);
}