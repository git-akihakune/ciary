#include "ciary.h"

int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month(int month, int year) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[month - 1];
}

int day_of_week(int year, int month, int day) {
    // Zeller's congruence algorithm
    if (month < 3) {
        month += 12;
        year--;
    }
    
    int century = year / 100;
    year = year % 100;
    
    int dow = (day + (13 * (month + 1)) / 5 + year + year / 4 + century / 4 - 2 * century) % 7;
    return (dow + 6) % 7; // Convert to 0=Sunday format
}

date_t get_current_date(void) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    
    date_t date;
    date.year = tm->tm_year + 1900;
    date.month = tm->tm_mon + 1;
    date.day = tm->tm_mday;
    
    return date;
}

void date_add_days(date_t *date, int days) {
    date->day += days;
    
    while (date->day > days_in_month(date->month, date->year)) {
        date->day -= days_in_month(date->month, date->year);
        date->month++;
        if (date->month > 12) {
            date->month = 1;
            date->year++;
        }
    }
    
    while (date->day < 1) {
        date->month--;
        if (date->month < 1) {
            date->month = 12;
            date->year--;
        }
        date->day += days_in_month(date->month, date->year);
    }
}

int date_compare(date_t a, date_t b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

void draw_help(void) {
    clear();
    
    mvprintw(2, 2, "CIARY - TUI Diary Help");
    mvprintw(4, 2, "Calendar Mode:");
    mvprintw(5, 4, "Arrow keys    - Navigate dates");
    mvprintw(6, 4, "Enter         - Edit entry for selected date");
    mvprintw(7, 4, "n             - Create new entry for today");
    mvprintw(8, 4, "v             - Change view mode (month/week/year)");
    mvprintw(9, 4, "h             - Show this help");
    mvprintw(10, 4, "q             - Quit application");
    
    mvprintw(12, 2, "Editor Mode:");
    mvprintw(13, 4, "Arrow keys    - Move cursor");
    mvprintw(14, 4, "Typing        - Insert text");
    mvprintw(15, 4, "Backspace     - Delete character");
    mvprintw(16, 4, "Enter         - New line");
    mvprintw(17, 4, "Ctrl+S        - Save entry");
    mvprintw(18, 4, "Ctrl+X        - Save and return to calendar");
    mvprintw(19, 4, "Ctrl+C        - Cancel and return to calendar");
    
    mvprintw(21, 2, "Notes:");
    mvprintw(22, 4, "- Entries are stored as Markdown files in ~/.ciary/");
    mvprintw(23, 4, "- Dates with entries are shown in bold");
    mvprintw(24, 4, "- Automatic timestamps are added to entries");
    
    mvprintw(26, 2, "Press any key to return...");
    refresh();
    getch();
}

void draw_status_bar(app_state_t *state) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    
    // Clear status line
    move(rows - 1, 0);
    clrtoeol();
    
    // Draw status information
    char status[256];
    if (state->mode == MODE_CALENDAR) {
        int entry_count = count_entries(state->selected_date);
        if (entry_count == 0) {
            snprintf(status, sizeof(status), "Calendar | Selected: %04d-%02d-%02d | No entry",
                    state->selected_date.year, state->selected_date.month, state->selected_date.day);
        } else {
            snprintf(status, sizeof(status), "Calendar | Selected: %04d-%02d-%02d | %d %s",
                    state->selected_date.year, state->selected_date.month, state->selected_date.day,
                    entry_count, (entry_count == 1) ? "entry" : "entries");
        }
    } else if (state->mode == MODE_EDITOR) {
        snprintf(status, sizeof(status), "Editor | Row: %d Col: %d | Editing: %04d-%02d-%02d",
                state->cursor_row + 1, state->cursor_col + 1,
                state->current_entry.date.year,
                state->current_entry.date.month,
                state->current_entry.date.day);
    }
    
    mvprintw(rows - 1, 0, "%s", status);
    
    // Right-aligned help text
    const char *help_text = "[h] Help";
    mvprintw(rows - 1, cols - strlen(help_text) - 1, "%s", help_text);
}