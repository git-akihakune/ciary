#include "ciary.h"
#include <signal.h>

// Global flag for interrupt handling
static volatile int interrupt_received = 0;

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    (void)sig; // Suppress unused parameter warning
    // Only set flag if not already set (avoid multiple interrupts)
    if (!interrupt_received) {
        interrupt_received = 1;
    }
}

// Show exit confirmation dialog
int show_exit_confirmation(void) {
    // Temporarily disable signal handler to prevent multiple interrupts
    signal(SIGINT, SIG_IGN);
    
    // Clear screen for dialog
    clear();
    
    // Display confirmation dialog
    int dialog_width = 50;
    int dialog_height = 8;
    int start_row = (LINES - dialog_height) / 2;
    int start_col = (COLS - dialog_width) / 2;
    
    // Draw border
    for (int i = 0; i < dialog_height; i++) {
        for (int j = 0; j < dialog_width; j++) {
            if (i == 0 || i == dialog_height - 1 || j == 0 || j == dialog_width - 1) {
                mvaddch(start_row + i, start_col + j, '*');
            } else {
                mvaddch(start_row + i, start_col + j, ' ');
            }
        }
    }
    
    // Display message
    mvprintw(start_row + 2, start_col + 2, "                EXIT CONFIRMATION");
    mvprintw(start_row + 4, start_col + 2, "   Are you sure you want to quit Ciary?");
    mvprintw(start_row + 5, start_col + 2, "        Press 'y' to quit, any other key to cancel");
    
    refresh();
    
    // Get user response - loop until we get valid input
    int ch;
    do {
        ch = getch();
    } while (ch == ERR);
    
    // Reset interrupt flag and restore signal handler
    interrupt_received = 0;
    signal(SIGINT, handle_sigint);
    
    return (ch == 'y' || ch == 'Y');
}

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
    
    // Set up signal handler for graceful Ctrl+C handling
    signal(SIGINT, handle_sigint);
}

void cleanup_app(void) {
    // Restore default signal handler
    signal(SIGINT, SIG_DFL);
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
        
        // Check for interrupt signal (Ctrl+C) before getting input
        if (interrupt_received) {
            if (show_exit_confirmation()) {
                break;
            }
            // If user cancelled, redraw and continue
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