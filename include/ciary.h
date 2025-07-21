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
#define CIARY_DIR ".ciary"

typedef enum {
    VIEW_MONTH,
    VIEW_WEEK,
    VIEW_YEAR
} view_mode_t;

typedef enum {
    MODE_CALENDAR,
    MODE_EDITOR,
    MODE_HELP
} app_mode_t;

typedef struct {
    int year;
    int month;
    int day;
} date_t;

typedef struct {
    date_t date;
    char content[MAX_CONTENT_SIZE];
    time_t created_time;
    time_t modified_time;
} entry_t;

typedef struct {
    app_mode_t mode;
    view_mode_t view;
    date_t current_date;
    date_t selected_date;
    entry_t current_entry;
    int cursor_row;
    int cursor_col;
    int scroll_offset;
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

// Editor functions
void draw_editor(app_state_t *state);
void handle_editor_input(app_state_t *state, int ch);
void insert_char(char *content, int pos, char ch);
void delete_char(char *content, int pos);

// File I/O functions
int ensure_ciary_dir(void);
char* get_entry_path(date_t date, time_t timestamp, char *path);
char* get_entry_path_legacy(date_t date, char *path);
int load_entry(date_t date, entry_t *entry);
int load_latest_entry(date_t date, entry_t *entry);
int save_entry(const entry_t *entry);
int entry_exists(date_t date);
int count_entries(date_t date);

// Utility functions
date_t get_current_date(void);
void date_add_days(date_t *date, int days);
int date_compare(date_t a, date_t b);
void draw_help(void);
void draw_status_bar(app_state_t *state);

#endif