#include "ciary.h"

static const char *month_names[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const char *day_names[] = {
    "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
};

void draw_calendar(app_state_t *state) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    
    clear();
    
    // Draw title
    int title_row = 2;
    char title[64];
    snprintf(title, sizeof(title), "%s %d", 
             month_names[state->current_date.month - 1], 
             state->current_date.year);
    mvprintw(title_row, (cols - strlen(title)) / 2, "%s", title);
    
    // Draw month calendar
    int start_row = 4;
    int start_col = (cols - 21) / 2; // 21 chars wide (3*7)
        
        // Draw day headers
        for (int i = 0; i < 7; i++) {
            mvprintw(start_row, start_col + i * 3, "%s", day_names[i]);
        }
        
        // Calculate first day of month
        int first_day = day_of_week(state->current_date.year, state->current_date.month, 1);
        int days = days_in_month(state->current_date.month, state->current_date.year);
        
        int day = 1;
        for (int week = 0; week < 6; week++) {
            int row = start_row + 2 + week;
            for (int dow = 0; dow < 7; dow++) {
                int col = start_col + dow * 3;
                
                if ((week == 0 && dow < first_day) || day > days) {
                    continue;
                }
                
                // Highlight current day and selected day
                bool is_selected = (day == state->selected_date.day &&
                                  state->current_date.month == state->selected_date.month &&
                                  state->current_date.year == state->selected_date.year);
                
                date_t check_date = {state->current_date.year, state->current_date.month, day};
                int entry_count = count_entries(check_date, &state->config);
                
                if (is_selected) {
                    attron(A_REVERSE);
                }
                if (entry_count > 0) {
                    attron(A_BOLD);
                }
                
                mvprintw(row, col, "%2d", day);
                
                if (is_selected) {
                    attroff(A_REVERSE);
                }
                if (entry_count > 0) {
                    attroff(A_BOLD);
                }
                
                day++;
            }
            if (day > days) break;
        }
    
    // Draw instructions - dynamic text based on editor
    const char* editor = get_actual_editor(&state->config);
    const char* new_text = (strcmp(editor, "nano") == 0) ? "Enter: New" : "n: New";
    char instructions[256];
    snprintf(instructions, sizeof(instructions), "Arrows: Navigate  %s  v: View  h: Help  q: Quit", new_text);
    mvprintw(rows - 3, 2, "%s", instructions);
    
    draw_status_bar(state);
    refresh();
}

void handle_calendar_input(app_state_t *state, int ch) {
    switch (ch) {
        case KEY_LEFT:
            if (state->selected_date.day > 1) {
                state->selected_date.day--;
            } else {
                // Previous month
                if (state->selected_date.month == 1) {
                    state->selected_date.month = 12;
                    state->selected_date.year--;
                } else {
                    state->selected_date.month--;
                }
                state->selected_date.day = days_in_month(state->selected_date.month, 
                                                       state->selected_date.year);
                state->current_date = state->selected_date;
            }
            break;
            
        case KEY_RIGHT:
            {
                int days = days_in_month(state->selected_date.month, state->selected_date.year);
                if (state->selected_date.day < days) {
                    state->selected_date.day++;
                } else {
                    // Next month
                    if (state->selected_date.month == 12) {
                        state->selected_date.month = 1;
                        state->selected_date.year++;
                    } else {
                        state->selected_date.month++;
                    }
                    state->selected_date.day = 1;
                    state->current_date = state->selected_date;
                }
            }
            break;
            
        case KEY_UP:
            if (state->selected_date.day > 7) {
                state->selected_date.day -= 7;
            }
            break;
            
        case KEY_DOWN:
            {
                int days = days_in_month(state->selected_date.month, state->selected_date.year);
                if (state->selected_date.day + 7 <= days) {
                    state->selected_date.day += 7;
                }
            }
            break;
            
        case '[':
        case KEY_PPAGE: // Page Up
            // Previous month
            {
                if (state->current_date.month == 1) {
                    state->current_date.month = 12;
                    state->current_date.year--;
                } else {
                    state->current_date.month--;
                }
                // Adjust selected day if it doesn't exist in the new month
                int max_days = days_in_month(state->current_date.month, state->current_date.year);
                if (state->selected_date.day > max_days) {
                    state->selected_date.day = max_days;
                }
                state->selected_date.month = state->current_date.month;
                state->selected_date.year = state->current_date.year;
            }
            break;
            
        case ']':
        case KEY_NPAGE: // Page Down
            // Next month
            {
                if (state->current_date.month == 12) {
                    state->current_date.month = 1;
                    state->current_date.year++;
                } else {
                    state->current_date.month++;
                }
                // Adjust selected day if it doesn't exist in the new month
                int max_days = days_in_month(state->current_date.month, state->current_date.year);
                if (state->selected_date.day > max_days) {
                    state->selected_date.day = max_days;
                }
                state->selected_date.month = state->current_date.month;
                state->selected_date.year = state->current_date.year;
            }
            break;
            
        case '<':
        case ',':
            // Previous year
            {
                state->current_date.year--;
                state->selected_date.year = state->current_date.year;
                // Handle leap year edge case for Feb 29
                int max_days = days_in_month(state->current_date.month, state->current_date.year);
                if (state->selected_date.day > max_days) {
                    state->selected_date.day = max_days;
                }
            }
            break;
            
        case '>':
        case '.':
            // Next year
            {
                state->current_date.year++;
                state->selected_date.year = state->current_date.year;
                // Handle leap year edge case for Feb 29
                int max_days = days_in_month(state->current_date.month, state->current_date.year);
                if (state->selected_date.day > max_days) {
                    state->selected_date.day = max_days;
                }
            }
            break;
            
        case '\n':
        case '\r':
        case KEY_ENTER:
        case 'n':
            // Open entry in external editor with appropriate time handling
            if (is_today(state->selected_date)) {
                // Today - use current time
                open_entry_in_editor(state->selected_date, &state->config);
            } else {
                // Other day - ask for time
                int hour, minute, second;
                if (prompt_for_time(&hour, &minute, &second) == 0) {
                    open_entry_with_time(state->selected_date, hour, minute, second, &state->config);
                }
            }
            break;
            
        case 'v':
            // View entry in read-only mode
            view_entry(state->selected_date, &state->config);
            break;
            
        case 'e':
            // Export entries
            {
                export_options_t options;
                if (show_export_dialog(state, &options)) {
                    export_entries(&options, &state->config);
                }
            }
            break;
    }
}