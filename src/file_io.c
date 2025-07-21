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

char* get_entry_path(date_t date, time_t timestamp, char *path) {
    char *home = getenv("HOME");
    if (!home) return NULL;
    
    snprintf(path, MAX_PATH_SIZE, "%s/%s/%04d-%02d-%02d_%ld.md",
             home, CIARY_DIR, date.year, date.month, date.day, timestamp);
    return path;
}

char* get_entry_path_legacy(date_t date, char *path) {
    char *home = getenv("HOME");
    if (!home) return NULL;
    
    snprintf(path, MAX_PATH_SIZE, "%s/%s/%04d-%02d-%02d.md",
             home, CIARY_DIR, date.year, date.month, date.day);
    return path;
}

int load_entry(date_t date, entry_t *entry) {
    // Try to load legacy format first (for backwards compatibility)
    char path[MAX_PATH_SIZE];
    if (!get_entry_path_legacy(date, path)) return -1;
    
    FILE *file = fopen(path, "r");
    if (!file) {
        // No legacy entry, create new empty entry
        memset(entry, 0, sizeof(entry_t));
        entry->date = date;
        entry->created_time = time(NULL);
        entry->modified_time = entry->created_time;
        return 0;
    }
    
    entry->date = date;
    
    // Read timestamp from first line if it exists
    char line[MAX_LINE_SIZE];
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "<!-- Created:", 13) == 0) {
            sscanf(line, "<!-- Created: %ld -->", &entry->created_time);
            if (fgets(line, sizeof(line), file) && 
                strncmp(line, "<!-- Modified:", 14) == 0) {
                sscanf(line, "<!-- Modified: %ld -->", &entry->modified_time);
            }
        } else {
            // No timestamp, rewind and read content
            rewind(file);
            entry->created_time = time(NULL);
            entry->modified_time = entry->created_time;
        }
    }
    
    // Read content
    size_t pos = 0;
    while (fgets(line, sizeof(line), file) && pos < MAX_CONTENT_SIZE - 1) {
        size_t len = strlen(line);
        if (pos + len >= MAX_CONTENT_SIZE - 1) break;
        strcpy(entry->content + pos, line);
        pos += len;
    }
    
    fclose(file);
    return 0;
}

int load_latest_entry(date_t date, entry_t *entry) {
    char *home = getenv("HOME");
    if (!home) return -1;
    
    char dir_path[MAX_PATH_SIZE];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", home, CIARY_DIR);
    
    DIR *dir = opendir(dir_path);
    if (!dir) return load_entry(date, entry);
    
    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%04d-%02d-%02d", date.year, date.month, date.day);
    
    time_t latest_time = 0;
    char latest_file[MAX_PATH_SIZE] = "";
    
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, prefix, strlen(prefix)) == 0) {
            // Check if it's a timestamped entry
            char *underscore = strchr(ent->d_name, '_');
            if (underscore) {
                time_t timestamp = atol(underscore + 1);
                if (timestamp > latest_time) {
                    latest_time = timestamp;
                    int path_len = strlen(dir_path) + strlen(ent->d_name) + 2; // +2 for '/' and '\0'
                    if (path_len <= MAX_PATH_SIZE) {
                        snprintf(latest_file, sizeof(latest_file), "%s/%s", dir_path, ent->d_name);
                    }
                }
            }
        }
    }
    closedir(dir);
    
    if (latest_time == 0) {
        // No timestamped entries, try legacy format
        return load_entry(date, entry);
    }
    
    // Load the latest timestamped entry
    FILE *file = fopen(latest_file, "r");
    if (!file) return -1;
    
    entry->date = date;
    entry->created_time = latest_time;
    entry->modified_time = latest_time;
    
    // Read content
    char line[MAX_LINE_SIZE];
    size_t pos = 0;
    
    // Skip timestamp comments if present
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "<!--", 4) == 0) {
            if (strncmp(line, "<!-- Created:", 13) == 0) {
                sscanf(line, "<!-- Created: %ld -->", &entry->created_time);
                continue;
            } else if (strncmp(line, "<!-- Modified:", 14) == 0) {
                sscanf(line, "<!-- Modified: %ld -->", &entry->modified_time);
                continue;
            }
        }
        
        // Regular content line
        size_t len = strlen(line);
        if (pos + len >= MAX_CONTENT_SIZE - 1) break;
        strcpy(entry->content + pos, line);
        pos += len;
    }
    
    fclose(file);
    return 0;
}

int save_entry(const entry_t *entry) {
    if (ensure_ciary_dir() == -1) return -1;
    
    char path[MAX_PATH_SIZE];
    time_t save_time = time(NULL);
    
    // Use creation time if it exists, otherwise current time
    time_t timestamp = (entry->created_time > 0) ? entry->created_time : save_time;
    
    if (!get_entry_path(entry->date, timestamp, path)) return -1;
    
    FILE *file = fopen(path, "w");
    if (!file) return -1;
    
    // Write timestamps with precise formatting
    fprintf(file, "<!-- Created: %ld -->\n", timestamp);
    fprintf(file, "<!-- Modified: %ld -->\n", save_time);
    
    // Write content
    if (strlen(entry->content) > 0) {
        fputs(entry->content, file);
        // Ensure file ends with newline
        if (entry->content[strlen(entry->content) - 1] != '\n') {
            fputc('\n', file);
        }
    }
    
    fclose(file);
    return 0;
}

int entry_exists(date_t date) {
    return count_entries(date) > 0;
}

int count_entries(date_t date) {
    char *home = getenv("HOME");
    if (!home) return 0;
    
    char dir_path[MAX_PATH_SIZE];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", home, CIARY_DIR);
    
    DIR *dir = opendir(dir_path);
    if (!dir) return 0;
    
    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%04d-%02d-%02d", date.year, date.month, date.day);
    
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, prefix, strlen(prefix)) == 0) {
            // Check if it ends with .md
            char *ext = strrchr(ent->d_name, '.');
            if (ext && strcmp(ext, ".md") == 0) {
                count++;
            }
        }
    }
    closedir(dir);
    
    return count;
}