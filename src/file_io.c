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

int open_entry_in_editor(date_t date, const config_t *config) {
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
    
    // If user has a specific preference, try that first
    if (strcmp(config->editor_preference, "auto") != 0) {
        snprintf(command, sizeof(command), "which %s >/dev/null 2>&1", config->editor_preference);
        if (system(command) == 0) {
            snprintf(command, sizeof(command), "%s \"%s\"", config->editor_preference, path);
            
            endwin();
            int result = system(command);
            
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            curs_set(1);
            
            return (result == 0) ? 0 : -1;
        }
    }
    
    // Fall back to auto-detection
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

int view_entry(date_t date, const config_t *config) {
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path)) return -1;
    
    // Check if file exists
    if (!entry_exists(date)) {
        // Show message that no entries exist for this date
        endwin();
        printf("No entries found for %04d-%02d-%02d\n", date.year, date.month, date.day);
        printf("Press Enter to continue...");
        getchar();
        
        // Reinitialize ncurses
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(1);
        return 0;
    }
    
    // Try different pagers in order of preference
    char command[MAX_PATH_SIZE + 50];
    const char *pagers[] = {"less", "more", "cat", NULL};
    
    // If user has a specific preference, try that first
    if (strcmp(config->viewer_preference, "auto") != 0) {
        snprintf(command, sizeof(command), "which %s >/dev/null 2>&1", config->viewer_preference);
        if (system(command) == 0) {
            if (strcmp(config->viewer_preference, "cat") == 0) {
                snprintf(command, sizeof(command), "%s \"%s\" && echo \"\\nPress Enter to continue...\" && read", config->viewer_preference, path);
            } else {
                snprintf(command, sizeof(command), "%s \"%s\"", config->viewer_preference, path);
            }
            
            endwin();
            int result = system(command);
            
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            curs_set(1);
            
            return (result == 0) ? 0 : -1;
        }
    }
    
    // Fall back to auto-detection
    for (int i = 0; pagers[i] != NULL; i++) {
        // Check if pager exists
        snprintf(command, sizeof(command), "which %s >/dev/null 2>&1", pagers[i]);
        if (system(command) == 0) {
            // Pager found, use it
            if (strcmp(pagers[i], "cat") == 0) {
                // For cat, add a pause after display
                snprintf(command, sizeof(command), "%s \"%s\" && echo \"\\nPress Enter to continue...\" && read", pagers[i], path);
            } else {
                // For less/more, they handle their own paging
                snprintf(command, sizeof(command), "%s \"%s\"", pagers[i], path);
            }
            
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
    
    return -1; // No suitable pager found
}