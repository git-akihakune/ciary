#include "ciary.h"

void init_app(app_state_t *state) {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    
    // Initialize application state
    state->mode = MODE_CALENDAR;
    state->view = VIEW_MONTH;
    state->current_date = get_current_date();
    state->selected_date = state->current_date;
    state->cursor_row = 0;
    state->cursor_col = 0;
    state->scroll_offset = 0;
    
    // Initialize current entry
    memset(&state->current_entry, 0, sizeof(entry_t));
    
    // Ensure ciary directory exists
    ensure_ciary_dir();
}

void cleanup_app(void) {
    endwin();
}

void run_app(app_state_t *state) {
    int ch;
    
    while (1) {
        switch (state->mode) {
            case MODE_CALENDAR:
                draw_calendar(state);
                break;
            case MODE_EDITOR:
                draw_editor(state);
                break;
            case MODE_HELP:
                draw_help();
                state->mode = MODE_CALENDAR;
                continue;
        }
        
        ch = getch();
        
        // Global commands
        if (ch == 'q' && state->mode == MODE_CALENDAR) {
            break;
        }
        if (ch == 'h') {
            state->mode = MODE_HELP;
            continue;
        }
        
        // Mode-specific input handling
        switch (state->mode) {
            case MODE_CALENDAR:
                handle_calendar_input(state, ch);
                break;
            case MODE_EDITOR:
                handle_editor_input(state, ch);
                break;
            default:
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv;
    
    app_state_t state;
    
    init_app(&state);
    
    // Show welcome message briefly
    clear();
    mvprintw(LINES / 2, (COLS - 20) / 2, "Welcome to Ciary!");
    mvprintw(LINES / 2 + 1, (COLS - 25) / 2, "Press any key to start...");
    refresh();
    getch();
    
    run_app(&state);
    
    cleanup_app();
    
    printf("Thank you for using Ciary!\n");
    return 0;
}