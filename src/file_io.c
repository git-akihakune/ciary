#include "ciary.h"
#include <dirent.h>

int ensure_ciary_dir(void) {
    char path[MAX_PATH_SIZE];
    char *home = getenv("HOME");
    if (!home) return -1;
    
    snprintf(path, sizeof(path), "%s/%s", home, CIARY_DIR);
    
    struct stat st;
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            return -1;
        }
    }
    return 0;
}

char* get_entry_path(date_t date, char *path) {
    char *home = getenv("HOME");
    if (!home) return NULL;
    
    snprintf(path, MAX_PATH_SIZE, "%s/%s/%04d-%02d-%02d.md",
             home, CIARY_DIR, date.year, date.month, date.day);
    return path;
}

int entry_exists(date_t date) {
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path)) return 0;
    
    struct stat st;
    return (stat(path, &st) == 0);
}

int count_entries(date_t date) {
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path)) return 0;
    
    FILE *file = fopen(path, "r");
    if (!file) return 0;
    
    int count = 0;
    char line[MAX_LINE_SIZE];
    
    while (fgets(line, sizeof(line), file)) {
        // Count lines that start with "## " (time headers)
        if (strncmp(line, "## ", 3) == 0) {
            count++;
        }
    }
    
    fclose(file);
    return count;
}

int open_entry_in_editor(date_t date) {
    if (ensure_ciary_dir() == -1) return -1;
    
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path)) return -1;
    
    // Add new entry with current time if file doesn't exist or is being edited
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    
    FILE *file = fopen(path, "a");
    if (!file) return -1;
    
    // Check if file is empty (new file)
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    
    if (size == 0) {
        // New file, add date header
        fprintf(file, "# %04d-%02d-%02d\n\n", date.year, date.month, date.day);
    } else {
        // Existing file, add some spacing
        fprintf(file, "\n");
    }
    
    // Add time header for new entry
    fprintf(file, "## %02d:%02d:%02d\n\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    fclose(file);
    
    // Try different editors in order of preference
    char command[MAX_PATH_SIZE + 50];
    const char *editors[] = {"nvim", "vim", "nano", "emacs", "vi", NULL};
    
    for (int i = 0; editors[i] != NULL; i++) {
        // Check if editor exists
        snprintf(command, sizeof(command), "which %s >/dev/null 2>&1", editors[i]);
        if (system(command) == 0) {
            // Editor found, use it
            snprintf(command, sizeof(command), "%s \"%s\"", editors[i], path);
            
            // Temporarily restore terminal settings
            endwin();
            int result = system(command);
            
            // Reinitialize ncurses
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            curs_set(1);
            
            return (result == 0) ? 0 : -1;
        }
    }
    
    return -1; // No suitable editor found
}