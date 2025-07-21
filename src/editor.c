#include "ciary.h"

void draw_editor(app_state_t *state) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    
    clear();
    
    // Draw title
    char title[64];
    snprintf(title, sizeof(title), "Entry for %04d-%02d-%02d",
             state->current_entry.date.year,
             state->current_entry.date.month,
             state->current_entry.date.day);
    mvprintw(1, (cols - strlen(title)) / 2, "%s", title);
    
    // Draw editor area
    int editor_start_row = 3;
    int editor_end_row = rows - 4;
    int editor_height = editor_end_row - editor_start_row;
    
    // Split content into lines for display
    char *content = state->current_entry.content;
    int content_len = strlen(content);
    int line_count = 0;
    int line_starts[1024];  // Max 1024 lines
    
    // Find line starts
    line_starts[0] = 0;
    line_count = 1;
    for (int i = 0; i < content_len && line_count < 1024; i++) {
        if (content[i] == '\n') {
            line_starts[line_count++] = i + 1;
        }
    }
    
    // Draw visible lines
    for (int i = 0; i < editor_height && (state->scroll_offset + i) < line_count; i++) {
        int line_idx = state->scroll_offset + i;
        int start = line_starts[line_idx];
        int end = (line_idx + 1 < line_count) ? line_starts[line_idx + 1] - 1 : content_len;
        
        // Extract line content
        char line[MAX_LINE_SIZE];
        int line_len = end - start;
        if (line_len >= MAX_LINE_SIZE) line_len = MAX_LINE_SIZE - 1;
        strncpy(line, content + start, line_len);
        line[line_len] = '\0';
        
        // Remove newline if present
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
        }
        
        mvprintw(editor_start_row + i, 1, "%s", line);
    }
    
    // Position cursor
    int cursor_line = 0;
    int char_count = 0;
    int target_pos = state->cursor_row * cols + state->cursor_col;
    
    // Find which line the cursor should be on
    for (int i = 0; i < content_len; i++) {
        if (char_count == target_pos) break;
        if (content[i] == '\n') {
            cursor_line++;
            char_count = 0;
        } else {
            char_count++;
        }
    }
    
    // Calculate visual cursor position
    int visual_row = cursor_line - state->scroll_offset + editor_start_row;
    int visual_col = state->cursor_col + 1;
    
    if (visual_row >= editor_start_row && visual_row < editor_end_row) {
        move(visual_row, visual_col);
    }
    
    // Draw instructions
    mvprintw(rows - 3, 2, "Ctrl+S: Save  Ctrl+X: Save & Exit  Ctrl+C: Cancel  Ctrl+H: Help");
    
    draw_status_bar(state);
    refresh();
}

void handle_editor_input(app_state_t *state, int ch) {
    int content_len = strlen(state->current_entry.content);
    
    switch (ch) {
        case KEY_LEFT:
            if (state->cursor_col > 0) {
                state->cursor_col--;
            } else if (state->cursor_row > 0) {
                state->cursor_row--;
                // Move to end of previous line
                int pos = state->cursor_row;
                while (pos < content_len && state->current_entry.content[pos] != '\n') {
                    state->cursor_col++;
                    pos++;
                }
            }
            break;
            
        case KEY_RIGHT:
            {
                int current_pos = state->cursor_row * COLS + state->cursor_col;
                if (current_pos < content_len) {
                    if (state->current_entry.content[current_pos] == '\n') {
                        state->cursor_row++;
                        state->cursor_col = 0;
                    } else {
                        state->cursor_col++;
                    }
                }
            }
            break;
            
        case KEY_UP:
            if (state->cursor_row > 0) {
                state->cursor_row--;
                if (state->cursor_row < state->scroll_offset) {
                    state->scroll_offset = state->cursor_row;
                }
            }
            break;
            
        case KEY_DOWN:
            state->cursor_row++;
            {
                int rows, cols;
                getmaxyx(stdscr, rows, cols);
                (void)cols; // Suppress unused warning
                int editor_height = rows - 7;  // Account for title and status
                if (state->cursor_row >= state->scroll_offset + editor_height) {
                    state->scroll_offset = state->cursor_row - editor_height + 1;
                }
            }
            break;
            
        case KEY_BACKSPACE:
        case 127:
        case '\b':
            {
                int pos = state->cursor_row * COLS + state->cursor_col;
                if (pos > 0) {
                    delete_char(state->current_entry.content, pos - 1);
                    if (state->cursor_col > 0) {
                        state->cursor_col--;
                    } else if (state->cursor_row > 0) {
                        state->cursor_row--;
                        // Find end of previous line
                        state->cursor_col = 0;
                        int check_pos = state->cursor_row * COLS;
                        while (check_pos < content_len - 1 && 
                               state->current_entry.content[check_pos] != '\n') {
                            state->cursor_col++;
                            check_pos++;
                        }
                    }
                }
            }
            break;
            
        case '\n':
        case '\r':
        case KEY_ENTER:
            {
                int pos = state->cursor_row * COLS + state->cursor_col;
                insert_char(state->current_entry.content, pos, '\n');
                state->cursor_row++;
                state->cursor_col = 0;
            }
            break;
            
        case 19: // Ctrl+S
            save_entry(&state->current_entry);
            break;
            
        case 24: // Ctrl+X
            save_entry(&state->current_entry);
            state->mode = MODE_CALENDAR;
            break;
            
        case 3: // Ctrl+C
            state->mode = MODE_CALENDAR;
            break;
            
        default:
            if (ch >= 32 && ch <= 126) {  // Printable characters
                int pos = state->cursor_row * COLS + state->cursor_col;
                insert_char(state->current_entry.content, pos, ch);
                state->cursor_col++;
            }
            break;
    }
}

void insert_char(char *content, int pos, char ch) {
    int len = strlen(content);
    if (len >= MAX_CONTENT_SIZE - 1) return;
    
    // Shift characters to the right
    for (int i = len; i > pos; i--) {
        content[i] = content[i - 1];
    }
    
    content[pos] = ch;
    content[len + 1] = '\0';
}

void delete_char(char *content, int pos) {
    int len = strlen(content);
    if (pos >= len) return;
    
    // Shift characters to the left
    for (int i = pos; i < len; i++) {
        content[i] = content[i + 1];
    }
}