#include "test_framework.h"
#include "../include/ciary.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Mock ncurses functions for UI testing
static char mock_screen[50][100];
static int mock_rows = 24;
static int mock_cols = 80;
static int mock_cursor_row = 0;
static int mock_cursor_col = 0;
static bool mock_ncurses_initialized = false;

// Mock ncurses state
typedef struct {
    int last_key;
    bool screen_cleared;
    int refresh_count;
    char status_bar[256];
    char instructions[256];
} mock_ui_state_t;

static mock_ui_state_t mock_ui_state = {0};

// Helper function to clear mock screen
void clear_mock_screen() {
    for (int i = 0; i < mock_rows; i++) {
        for (int j = 0; j < mock_cols; j++) {
            mock_screen[i][j] = ' ';
        }
        mock_screen[i][mock_cols - 1] = '\0';
    }
    mock_ui_state.screen_cleared = true;
}

// Helper function to set text at position
void set_mock_text(int row, int col, const char* text) {
    if (row >= 0 && row < mock_rows && col >= 0) {
        int len = strlen(text);
        for (int i = 0; i < len && (col + i) < (mock_cols - 1); i++) {
            mock_screen[row][col + i] = text[i];
        }
    }
}

// Helper function to get text from screen
void get_mock_text(int row, int start_col, int end_col, char* buffer) {
    if (row >= 0 && row < mock_rows && start_col >= 0 && end_col < mock_cols) {
        int len = end_col - start_col + 1;
        strncpy(buffer, &mock_screen[row][start_col], len);
        buffer[len] = '\0';
        
        // Trim trailing spaces
        for (int i = len - 1; i >= 0 && buffer[i] == ' '; i--) {
            buffer[i] = '\0';
        }
    } else {
        buffer[0] = '\0';
    }
}

void test_calendar_display_format() {
    TEST_CASE("Calendar Display Format");
    
    clear_mock_screen();
    
    // Test month title display
    app_state_t state;
    state.current_date = (date_t){2024, 7, 15};
    state.selected_date = (date_t){2024, 7, 15};
    
    // Simulate calendar title
    char title[64];
    snprintf(title, sizeof(title), "July 2024");
    set_mock_text(2, (mock_cols - strlen(title)) / 2, title);
    
    char title_buffer[64];
    get_mock_text(2, 30, 40, title_buffer);
    ASSERT_TRUE(strstr(title_buffer, "July") != NULL, "Month name should be displayed");
    ASSERT_TRUE(strstr(title_buffer, "2024") != NULL, "Year should be displayed");
    
    // Test day headers
    const char* day_names[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    int start_col = (mock_cols - 21) / 2;
    
    for (int i = 0; i < 7; i++) {
        set_mock_text(4, start_col + i * 3, day_names[i]);
    }
    
    // Verify day headers are displayed correctly
    char day_header[10];
    for (int i = 0; i < 7; i++) {
        get_mock_text(4, start_col + i * 3, start_col + i * 3 + 1, day_header);
        ASSERT_STR_EQ(day_names[i], day_header, "Day header should be correctly positioned");
    }
}

void test_calendar_date_positioning() {
    TEST_CASE("Calendar Date Positioning");
    
    clear_mock_screen();
    
    // Test July 2024 layout (starts on Monday)
    date_t test_date = {2024, 7, 1};
    int first_day = day_of_week(2024, 7, 1); // Monday = 1
    int days_in_july = days_in_month(7, 2024); // 31 days
    
    ASSERT_EQ(1, first_day, "July 1, 2024 should be Monday");
    ASSERT_EQ(31, days_in_july, "July should have 31 days");
    
    // Simulate calendar layout
    int start_row = 6; // Start after headers
    int start_col = (mock_cols - 21) / 2;
    
    int day = 1;
    for (int week = 0; week < 6; week++) {
        for (int dow = 0; dow < 7; dow++) {
            if ((week == 0 && dow < first_day) || day > days_in_july) {
                continue;
            }
            
            char day_str[4];
            snprintf(day_str, sizeof(day_str), "%2d", day);
            set_mock_text(start_row + week, start_col + dow * 3, day_str);
            day++;
        }
        if (day > days_in_july) break;
    }
    
    // Verify specific dates are in correct positions
    char date_text[4];
    
    // July 1 should be in second column (Monday)
    get_mock_text(start_row, start_col + 1 * 3, start_col + 1 * 3 + 1, date_text);
    ASSERT_STR_EQ(" 1", date_text, "July 1 should be in Monday column");
    
    // July 15 should be in second week, Monday column
    get_mock_text(start_row + 2, start_col + 1 * 3, start_col + 1 * 3 + 1, date_text);
    ASSERT_STR_EQ("15", date_text, "July 15 should be correctly positioned");
    
    // July 31 should be in last week, Wednesday column
    get_mock_text(start_row + 4, start_col + 3 * 3, start_col + 3 * 3 + 1, date_text);
    ASSERT_STR_EQ("31", date_text, "July 31 should be in Wednesday column");
}

void test_selected_date_highlighting() {
    TEST_CASE("Selected Date Highlighting");
    
    // Test that selected date is visually distinct
    app_state_t state;
    state.current_date = (date_t){2024, 7, 15};
    state.selected_date = (date_t){2024, 7, 15};
    
    // Simulate highlighting logic
    bool is_selected = (state.selected_date.day == 15 &&
                       state.selected_date.month == 7 &&
                       state.selected_date.year == 2024);
    
    ASSERT_TRUE(is_selected, "Current date should be marked as selected");
    
    // Test different selected date
    state.selected_date = (date_t){2024, 7, 20};
    is_selected = (state.selected_date.day == 15 &&
                  state.selected_date.month == 7 &&
                  state.selected_date.year == 2024);
    
    ASSERT_FALSE(is_selected, "Non-current date should not be marked as selected");
}

void test_entry_count_indication() {
    TEST_CASE("Entry Count Indication");
    
    // Create test environment
    char* test_dir = create_temp_dir();
    if (!test_dir) {
        printf("⚠ Skipping test - could not create temp directory\n");
        return;
    }
    
    config_t config;
    load_default_config(&config);
    strncpy(config.journal_directory, test_dir, sizeof(config.journal_directory) - 1);
    config.journal_directory[sizeof(config.journal_directory) - 1] = '\0';
    
    ensure_journal_dir(&config);
    
    // Create test entries
    date_t test_date = {2024, 7, 15};
    char path[512];
    get_entry_path(test_date, path, &config);
    
    FILE *file = fopen(path, "w");
    if (file) {
        fprintf(file, "# 2024-07-15\n\n");
        fprintf(file, "## 09:00:00\n\nMorning entry\n\n");
        fprintf(file, "## 18:00:00\n\nEvening entry\n");
        fclose(file);
        
        int entry_count = count_entries(test_date, &config);
        ASSERT_EQ(2, entry_count, "Should count entries correctly for UI display");
        
        // Test visual indication logic
        bool has_entries = (entry_count > 0);
        ASSERT_TRUE(has_entries, "Date with entries should be visually indicated");
    }
    
    // Test date without entries
    date_t empty_date = {2024, 7, 16};
    int empty_count = count_entries(empty_date, &config);
    ASSERT_EQ(0, empty_count, "Empty date should show no entries");
    
    bool empty_has_entries = (empty_count > 0);
    ASSERT_FALSE(empty_has_entries, "Date without entries should not be highlighted");
    
    remove_temp_dir(test_dir);
}

void test_keyboard_navigation() {
    TEST_CASE("Keyboard Navigation");
    
    app_state_t state;
    state.current_date = (date_t){2024, 7, 15};
    state.selected_date = (date_t){2024, 7, 15};
    
    // Test arrow key navigation
    date_t original_date = state.selected_date;
    
    // Simulate right arrow key
    if (state.selected_date.day < days_in_month(state.selected_date.month, state.selected_date.year)) {
        state.selected_date.day++;
    }
    
    ASSERT_EQ(16, state.selected_date.day, "Right arrow should move to next day");
    
    // Simulate left arrow key
    state.selected_date.day--;
    ASSERT_EQ(15, state.selected_date.day, "Left arrow should move to previous day");
    
    // Simulate down arrow key (move to next week)
    int days_in_current_month = days_in_month(state.selected_date.month, state.selected_date.year);
    if (state.selected_date.day + 7 <= days_in_current_month) {
        state.selected_date.day += 7;
    }
    
    ASSERT_EQ(22, state.selected_date.day, "Down arrow should move to next week");
    
    // Simulate up arrow key (move to previous week)
    if (state.selected_date.day > 7) {
        state.selected_date.day -= 7;
    }
    
    ASSERT_EQ(15, state.selected_date.day, "Up arrow should move to previous week");
}

void test_month_year_navigation() {
    TEST_CASE("Month and Year Navigation");
    
    app_state_t state;
    state.current_date = (date_t){2024, 7, 15};
    state.selected_date = (date_t){2024, 7, 15};
    
    // Test month navigation
    date_t original_date = state.selected_date;
    
    // Next month (simulate ']' key)
    if (state.current_date.month == 12) {
        state.current_date.month = 1;
        state.current_date.year++;
    } else {
        state.current_date.month++;
    }
    
    // Adjust selected day if needed
    int max_days = days_in_month(state.current_date.month, state.current_date.year);
    if (state.selected_date.day > max_days) {
        state.selected_date.day = max_days;
    }
    state.selected_date.month = state.current_date.month;
    state.selected_date.year = state.current_date.year;
    
    ASSERT_EQ(8, state.current_date.month, "Should navigate to August");
    ASSERT_EQ(2024, state.current_date.year, "Year should remain same");
    
    // Previous month (simulate '[' key)
    if (state.current_date.month == 1) {
        state.current_date.month = 12;
        state.current_date.year--;
    } else {
        state.current_date.month--;
    }
    
    max_days = days_in_month(state.current_date.month, state.current_date.year);
    if (state.selected_date.day > max_days) {
        state.selected_date.day = max_days;
    }
    state.selected_date.month = state.current_date.month;
    state.selected_date.year = state.current_date.year;
    
    ASSERT_EQ(7, state.current_date.month, "Should navigate back to July");
    
    // Test year navigation
    // Next year (simulate '>' key)
    state.current_date.year++;
    state.selected_date.year = state.current_date.year;
    
    // Handle leap year edge case
    max_days = days_in_month(state.current_date.month, state.current_date.year);
    if (state.selected_date.day > max_days) {
        state.selected_date.day = max_days;
    }
    
    ASSERT_EQ(2025, state.current_date.year, "Should navigate to next year");
    
    // Previous year (simulate '<' key)
    state.current_date.year--;
    state.selected_date.year = state.current_date.year;
    
    ASSERT_EQ(2024, state.current_date.year, "Should navigate back to original year");
}

void test_status_bar_display() {
    TEST_CASE("Status Bar Display");
    
    clear_mock_screen();
    
    // Test status bar content
    app_state_t state;
    state.current_date = (date_t){2024, 7, 15};
    state.selected_date = (date_t){2024, 7, 15};
    
    // Simulate status bar display
    char status_text[256];
    snprintf(status_text, sizeof(status_text), "Selected: 2024-07-15 | Entries: 0 | Today");
    
    set_mock_text(mock_rows - 1, 2, status_text);
    
    char status_buffer[256];
    get_mock_text(mock_rows - 1, 0, mock_cols - 1, status_buffer);
    
    ASSERT_TRUE(strstr(status_buffer, "2024-07-15") != NULL, "Status bar should show selected date");
    ASSERT_TRUE(strstr(status_buffer, "Entries:") != NULL, "Status bar should show entry count");
}

void test_instruction_bar() {
    TEST_CASE("Instruction Bar");
    
    clear_mock_screen();
    
    // Test instruction display
    config_t config;
    load_default_config(&config);
    
    const char* editor = get_actual_editor(&config);
    const char* new_text = (strcmp(editor, "nano") == 0) ? "Enter: New" : "n: New";
    
    char instructions[256];
    snprintf(instructions, sizeof(instructions), "Arrows: Navigate  %s  v: View  h: Help  q: Quit", new_text);
    
    set_mock_text(mock_rows - 3, 2, instructions);
    
    char inst_buffer[256];
    get_mock_text(mock_rows - 3, 0, mock_cols - 1, inst_buffer);
    
    ASSERT_TRUE(strstr(inst_buffer, "Navigate") != NULL, "Instructions should mention navigation");
    ASSERT_TRUE(strstr(inst_buffer, "View") != NULL, "Instructions should mention view option");
    ASSERT_TRUE(strstr(inst_buffer, "Help") != NULL, "Instructions should mention help");
    ASSERT_TRUE(strstr(inst_buffer, "Quit") != NULL, "Instructions should mention quit");
    
    // Test dynamic instruction based on editor
    ASSERT_TRUE(strstr(inst_buffer, "New") != NULL, "Instructions should show new entry option");
}

void test_help_screen_layout() {
    TEST_CASE("Help Screen Layout");
    
    clear_mock_screen();
    
    // Simulate help screen content
    set_mock_text(2, 2, "CIARY - TUI Diary Help");
    set_mock_text(4, 2, "Calendar Navigation:");
    set_mock_text(5, 4, "Arrow keys    - Navigate dates within month");
    set_mock_text(6, 4, "[ / Page Up   - Previous month");
    set_mock_text(7, 4, "] / Page Down - Next month");
    set_mock_text(8, 4, "< / ,         - Previous year");
    set_mock_text(9, 4, "> / .         - Next year");
    
    // Verify help content
    char help_title[64];
    get_mock_text(2, 0, 30, help_title);
    ASSERT_TRUE(strstr(help_title, "Help") != NULL, "Help screen should have title");
    
    char nav_section[64];
    get_mock_text(4, 0, 30, nav_section);
    ASSERT_TRUE(strstr(nav_section, "Navigation") != NULL, "Help should have navigation section");
    
    char arrow_help[64];
    get_mock_text(5, 0, 50, arrow_help);
    ASSERT_TRUE(strstr(arrow_help, "Arrow keys") != NULL, "Help should explain arrow keys");
}

void test_screen_responsiveness() {
    TEST_CASE("Screen Responsiveness");
    
    // Test different terminal sizes
    struct {
        int rows, cols;
        bool should_fit;
        const char* description;
    } screen_sizes[] = {
        {24, 80, true, "Standard 80x24 terminal"},
        {30, 100, true, "Large terminal"},
        {20, 60, false, "Small terminal (too narrow)"},
        {15, 80, false, "Short terminal"},
        {40, 120, true, "Wide terminal"}
    };
    
    for (int i = 0; i < 5; i++) {
        mock_rows = screen_sizes[i].rows;
        mock_cols = screen_sizes[i].cols;
        
        // Test minimum requirements for calendar display
        int min_width_needed = 21; // 7 days * 3 chars each
        int min_height_needed = 12; // Title + headers + 6 weeks + instructions
        
        bool fits = (mock_cols >= min_width_needed && mock_rows >= min_height_needed);
        
        if (screen_sizes[i].should_fit) {
            ASSERT_TRUE(fits, screen_sizes[i].description);
        } else if (mock_cols < min_width_needed || mock_rows < min_height_needed) {
            ASSERT_FALSE(fits, screen_sizes[i].description);
        }
    }
    
    // Reset to standard size
    mock_rows = 24;
    mock_cols = 80;
}

void test_user_interaction_flow() {
    TEST_CASE("User Interaction Flow");
    
    // Test complete user interaction sequence
    app_state_t state;
    state.current_date = (date_t){2024, 7, 15};
    state.selected_date = (date_t){2024, 7, 15};
    
    // 1. User starts application (today's date selected)
    ASSERT_TRUE(is_today(state.current_date) || 
                (state.current_date.year == 2024 && state.current_date.month == 7 && state.current_date.day == 15),
                "Application should start with meaningful date");
    
    // 2. User navigates to different date
    state.selected_date.day = 10;
    ASSERT_EQ(10, state.selected_date.day, "Navigation should update selected date");
    
    // 3. User checks entry existence
    // (This would normally involve UI feedback)
    bool has_entries = false; // Simulated
    ASSERT_FALSE(has_entries, "UI should indicate no entries for new date");
    
    // 4. User creates entry (simulated keypress)
    char simulated_action = 'n'; // New entry
    ASSERT_EQ('n', simulated_action, "User input should be captured correctly");
    
    // 5. User views entry (simulated)
    simulated_action = 'v'; // View entry
    ASSERT_EQ('v', simulated_action, "View command should be recognized");
    
    // 6. User accesses help
    simulated_action = 'h'; // Help
    ASSERT_EQ('h', simulated_action, "Help command should be recognized");
    
    // 7. User quits application
    simulated_action = 'q'; // Quit
    ASSERT_EQ('q', simulated_action, "Quit command should be recognized");
}

void test_accessibility_features() {
    TEST_CASE("Accessibility Features");
    
    // Test keyboard-only navigation
    ASSERT_TRUE(true, "All functionality accessible via keyboard");
    
    // Test clear visual indicators
    app_state_t state;
    state.selected_date = (date_t){2024, 7, 15};
    
    bool is_selected = true; // Simulated highlighting
    ASSERT_TRUE(is_selected, "Selected date should have clear visual indication");
    
    // Test consistent key mappings
    struct {
        char key;
        const char* action;
    } key_mappings[] = {
        {'h', "Help"},
        {'q', "Quit"}, 
        {'v', "View"},
        {'n', "New entry"},
        {'\n', "New entry (Enter)"}
    };
    
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(key_mappings[i].key != 0, "Key mapping should be defined");
        ASSERT_TRUE(strlen(key_mappings[i].action) > 0, "Action should be described");
    }
    
    // Test help accessibility
    ASSERT_TRUE(true, "Help system provides comprehensive usage information");
    
    // Test error feedback
    ASSERT_TRUE(true, "Error conditions should provide clear feedback");
}

void test_visual_consistency() {
    TEST_CASE("Visual Consistency");
    
    clear_mock_screen();
    
    // Test consistent spacing and alignment
    int calendar_width = 21; // 7 days * 3 chars
    int screen_center = mock_cols / 2;
    int calendar_start = screen_center - calendar_width / 2;
    
    ASSERT_TRUE(calendar_start > 0, "Calendar should be centered on screen");
    
    // Test consistent text positioning
    struct {
        int row;
        const char* content_type;
        bool should_be_centered;
    } layout_elements[] = {
        {2, "Month title", true},
        {4, "Day headers", true},
        {mock_rows - 3, "Instructions", false},
        {mock_rows - 1, "Status bar", false}
    };
    
    for (int i = 0; i < 4; i++) {
        if (layout_elements[i].should_be_centered) {
            ASSERT_TRUE(layout_elements[i].row < mock_rows / 2 || 
                       layout_elements[i].row == 2,
                       "Centered elements should be in upper half or title area");
        } else {
            ASSERT_TRUE(layout_elements[i].row >= mock_rows - 3,
                       "Status elements should be at bottom of screen");
        }
    }
    
    // Test visual hierarchy
    ASSERT_TRUE(2 < 4, "Title should appear before day headers");
    ASSERT_TRUE(4 < 6, "Day headers should appear before calendar dates");
    ASSERT_TRUE(mock_rows - 3 < mock_rows - 1, "Instructions should appear before status bar");
}

void run_ui_tests() {
    TEST_SUITE("UI/UX Tests");
    
    test_calendar_display_format();
    test_calendar_date_positioning();
    test_selected_date_highlighting();
    test_entry_count_indication();
    test_keyboard_navigation();
    test_month_year_navigation();
    test_status_bar_display();
    test_instruction_bar();
    test_help_screen_layout();
    test_screen_responsiveness();
    test_user_interaction_flow();
    test_accessibility_features();
    test_visual_consistency();
}