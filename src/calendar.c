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
    
    // Draw view mode indicator
    const char *view_str = (state->view == VIEW_MONTH) ? "Month View" :
                          (state->view == VIEW_WEEK) ? "Week View" : "Year View";
    mvprintw(title_row + 1, (cols - strlen(view_str)) / 2, "%s", view_str);
    
    if (state->view == VIEW_MONTH) {
        // Draw month calendar
        int start_row = 5;
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
                int entry_count = count_entries(check_date);
                
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
    }
    
    // Draw instructions
    mvprintw(rows - 4, 2, "Arrow keys: Navigate  Enter: Edit entry  v: Change view");
    mvprintw(rows - 3, 2, "n: New entry  d: Delete entry  h: Help  q: Quit");
    
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
            
        case '\n':
        case '\r':
        case KEY_ENTER:
            // Load latest entry and switch to editor
            if (load_latest_entry(state->selected_date, &state->current_entry) == 0) {
                state->mode = MODE_EDITOR;
                state->cursor_row = 0;
                state->cursor_col = 0;
                state->scroll_offset = 0;
            }
            break;
            
        case 'v':
            // Cycle through view modes
            state->view = (state->view + 1) % 3;
            break;
            
        case 'n':
            // Create new entry for today
            state->selected_date = get_current_date();
            state->current_date = state->selected_date;
            // Create a completely new entry with current timestamp
            memset(&state->current_entry, 0, sizeof(entry_t));
            state->current_entry.date = state->selected_date;
            state->current_entry.created_time = time(NULL);
            state->current_entry.modified_time = state->current_entry.created_time;
            state->mode = MODE_EDITOR;
            state->cursor_row = 0;
            state->cursor_col = 0;
            state->scroll_offset = 0;
            break;
    }
}