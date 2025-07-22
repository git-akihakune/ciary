#include "ciary.h"
#include <dirent.h>
#include <stdlib.h>

int ensure_journal_dir(const config_t *config) {
    struct stat st;
    
    // Create directories recursively if they don't exist
    char *path_copy = malloc(strlen(config->journal_directory) + 1);
    strcpy(path_copy, config->journal_directory);
    
    char *p = path_copy;
    while (*p) {
        if (*p == '/' && p != path_copy) {
            *p = '\0';
            if (stat(path_copy, &st) == -1) {
                mkdir(path_copy, 0755);
            }
            *p = '/';
        }
        p++;
    }
    
    // Create the final directory
    if (stat(config->journal_directory, &st) == -1) {
        if (mkdir(config->journal_directory, 0755) == -1) {
            free(path_copy);
            return -1;
        }
    }
    
    free(path_copy);
    return 0;
}

char* get_entry_path(date_t date, char *path, const config_t *config) {
    snprintf(path, MAX_PATH_SIZE, "%s/%04d-%02d-%02d.md",
             config->journal_directory, date.year, date.month, date.day);
    return path;
}

int entry_exists(date_t date, const config_t *config) {
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path, config)) return 0;
    
    struct stat st;
    return (stat(path, &st) == 0);
}

int count_entries(date_t date, const config_t *config) {
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path, config)) return 0;
    
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
    if (ensure_journal_dir(config) == -1) return -1;
    
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path, config)) return -1;
    
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

int open_entry_with_time(date_t date, int hour, int minute, int second, const config_t *config) {
    if (ensure_journal_dir(config) == -1) return -1;
    
    char path[MAX_PATH_SIZE];
    if (!get_entry_path(date, path, config)) return -1;
    
    // Add new entry with specified time
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
    
    // Add time header for new entry with specified time
    fprintf(file, "## %02d:%02d:%02d\n\n", hour, minute, second);
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
    if (!get_entry_path(date, path, config)) return -1;
    
    // Check if file exists
    if (!entry_exists(date, config)) {
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