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
    app_mode_t mode;
    date_t current_date;
    date_t selected_date;
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
int ensure_ciary_dir(void);
char* get_entry_path(date_t date, char *path);
int entry_exists(date_t date);
int count_entries(date_t date);
int open_entry_in_editor(date_t date);
int view_entry(date_t date);

// Utility functions
date_t get_current_date(void);
void date_add_days(date_t *date, int days);
int date_compare(date_t a, date_t b);
void draw_help(void);
void draw_status_bar(app_state_t *state);

// Welcome message functions
char* get_username(void);
char* get_time_greeting(void);
char* get_season_info(void);
char* get_day_phase(void);
void generate_welcome_message(char *message, size_t size);
void show_personalized_welcome(void);
void show_personalized_goodbye(void);

#endif