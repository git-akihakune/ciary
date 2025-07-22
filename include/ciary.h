#ifndef CIARY_H
#define CIARY_H

#include <ncurses.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CONTENT_SIZE 8192
#define MAX_PATH_SIZE 512
#define MAX_LINE_SIZE 256
#define MAX_NAME_SIZE 64
#define CIARY_CONFIG_DIR ".config/ciary"
#define CIARY_DATA_DIR ".local/share/ciary"
#define CONFIG_FILE "config.conf"

// Removed view modes - only month view now

typedef enum {
    MODE_CALENDAR,
    MODE_HELP
} app_mode_t;

typedef struct {
    int year;
    int month;
    int day;
} date_t;

typedef struct {
    char preferred_name[MAX_NAME_SIZE];
    char editor_preference[MAX_NAME_SIZE];
    char viewer_preference[MAX_NAME_SIZE];
    char journal_directory[MAX_PATH_SIZE];
    int show_ascii_art;
    int enable_personalization;
} config_t;

typedef enum {
    EXPORT_FORMAT_HTML,
    EXPORT_FORMAT_PDF,
    EXPORT_FORMAT_MARKDOWN
} export_format_t;

typedef enum {
    DATE_RANGE_ALL,
    DATE_RANGE_LAST_7_DAYS,
    DATE_RANGE_THIS_MONTH,
    DATE_RANGE_THIS_YEAR,
    DATE_RANGE_CUSTOM
} date_range_preset_t;

typedef struct {
    date_t start_date;
    date_t end_date;
    export_format_t format;
    char output_path[MAX_PATH_SIZE];
    int include_empty_days;
} export_options_t;

typedef struct {
    app_mode_t mode;
    date_t current_date;
    date_t selected_date;
    config_t config;
} app_state_t;

// Function declarations
void init_app(app_state_t *state);
void cleanup_app(void);
void run_app(app_state_t *state);

// Calendar functions
void draw_calendar(app_state_t *state);
void handle_calendar_input(app_state_t *state, int ch);
int is_leap_year(int year);
int days_in_month(int month, int year);
int day_of_week(int year, int month, int day);

// File I/O functions
int ensure_journal_dir(const config_t *config);
char* get_entry_path(date_t date, char *path, const config_t *config);
int entry_exists(date_t date, const config_t *config);
int count_entries(date_t date, const config_t *config);
int open_entry_in_editor(date_t date, const config_t *config);
int open_entry_with_time(date_t date, int hour, int minute, int second, const config_t *config);
int view_entry(date_t date, const config_t *config);
int prompt_for_time(int *hour, int *minute, int *second);
int is_today(date_t date);
const char* get_actual_editor(const config_t *config);

// Utility functions
date_t get_current_date(void);
void date_add_days(date_t *date, int days);
int date_compare(date_t a, date_t b);
void draw_help(void);
void draw_status_bar(app_state_t *state);

// Config functions
int ensure_config_dir(void);
char* get_config_path(char *path);
void load_default_config(config_t *config);
int load_config(config_t *config);
int save_config(const config_t *config);
int setup_first_run(config_t *config);

// Welcome message functions
char* get_username(const config_t *config);
char* get_time_greeting(void);
char* get_season_info(void);
char* get_day_phase(void);
void generate_welcome_message(char *message, size_t size, const config_t *config);
void show_personalized_welcome(const config_t *config);
void show_personalized_goodbye(const config_t *config);

// Export functions
int show_export_dialog(app_state_t *state, export_options_t *options);
int export_entries(const export_options_t *options, const config_t *config);
int collect_entries_in_range(const export_options_t *options, const config_t *config, char ***entry_files, int *file_count);
int export_to_html(const export_options_t *options, const config_t *config, char **entry_files, int file_count);
int export_to_pdf(const export_options_t *options, const config_t *config, char **entry_files, int file_count);
int export_to_markdown(const export_options_t *options, const config_t *config, char **entry_files, int file_count);
void show_progress_bar(const char *message, int current, int total);
void calculate_date_range(date_range_preset_t preset, date_t current_date, date_t *start, date_t *end);

#endif