#include "ciary.h"

void init_app(app_state_t *state) {
    // Initialize application state
    state->mode = MODE_CALENDAR;
    state->current_date = get_current_date();
    state->selected_date = state->current_date;
    
    // Load configuration (handles first-run setup) - before ncurses
    setup_first_run(&state->config);
    
    // Initialize ncurses after config setup
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
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
    
    // Show personalized welcome message
    show_personalized_welcome(&state.config);
    
    run_app(&state);
    
    cleanup_app();
    
    show_personalized_goodbye(&state.config);
    return 0;
}