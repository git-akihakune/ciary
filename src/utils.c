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
    mvprintw(4, 2, "Calendar Navigation:");
    mvprintw(5, 4, "Arrow keys    - Navigate dates");
    mvprintw(6, 4, "Enter or n    - Create new entry (opens external editor)");
    mvprintw(7, 4, "v             - View existing entries (read-only)");
    mvprintw(8, 4, "h             - Show this help");
    mvprintw(9, 4, "q             - Quit application");
    
    mvprintw(11, 2, "Entry Format:");
    mvprintw(12, 4, "- One file per day: ~/.ciary/YYYY-MM-DD.md");
    mvprintw(13, 4, "- Multiple entries per day with time headers");
    mvprintw(14, 4, "- Format: ## HH:MM:SS followed by entry content");
    mvprintw(15, 4, "- Dates with entries are shown in bold");
    
    mvprintw(17, 2, "External Tools:");
    mvprintw(18, 4, "- Editors: nvim, vim, nano, emacs, vi (first available)");
    mvprintw(19, 4, "- Viewers: less, more, cat (first available)");
    
    mvprintw(21, 2, "Press any key to return...");
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
        int entry_count = count_entries(state->selected_date, &state->config);
        if (entry_count == 0) {
            snprintf(status, sizeof(status), "Calendar | Selected: %04d-%02d-%02d | No entry",
                    state->selected_date.year, state->selected_date.month, state->selected_date.day);
        } else {
            snprintf(status, sizeof(status), "Calendar | Selected: %04d-%02d-%02d | %d %s",
                    state->selected_date.year, state->selected_date.month, state->selected_date.day,
                    entry_count, (entry_count == 1) ? "entry" : "entries");
        }
    }
    
    mvprintw(rows - 1, 0, "%s", status);
    
    // Right-aligned help text
    const char *help_text = "[h] Help";
    mvprintw(rows - 1, cols - strlen(help_text) - 1, "%s", help_text);
}

// Welcome message system with personality
char* get_username(const config_t *config) {
    static char username[MAX_NAME_SIZE];
    strncpy(username, config->preferred_name, sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';
    return username;
}

char* get_time_greeting(void) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    int hour = tm->tm_hour;
    
    if (hour < 5) return "burning the midnight oil";
    else if (hour < 12) return "bright and early";
    else if (hour < 17) return "in the thick of the day";
    else if (hour < 21) return "as evening settles in";
    else return "as the night embraces us";
}

char* get_season_info(void) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    int month = tm->tm_mon + 1;
    int day = tm->tm_mday;
    
    // Northern hemisphere seasons (can be enhanced for location detection)
    if ((month == 12 && day >= 21) || month <= 2 || (month == 3 && day < 20)) {
        static char winter_msgs[][64] = {
            "winter's crisp embrace",
            "the frosty season",
            "winter's quiet wisdom",
            "the season of reflection",
            "winter's cozy sanctuary"
        };
        return winter_msgs[rand() % 5];
    }
    else if ((month == 3 && day >= 20) || month <= 5 || (month == 6 && day < 21)) {
        static char spring_msgs[][64] = {
            "spring's hopeful awakening",
            "the season of new beginnings",
            "spring's gentle renewal",
            "nature's grand resurrection",
            "the blooming season"
        };
        return spring_msgs[rand() % 5];
    }
    else if ((month == 6 && day >= 21) || month <= 8 || (month == 9 && day < 22)) {
        static char summer_msgs[][64] = {
            "summer's golden embrace",
            "the vibrant season",
            "summer's endless energy",
            "the sun-kissed days",
            "the season of adventure"
        };
        return summer_msgs[rand() % 5];
    }
    else {
        static char autumn_msgs[][64] = {
            "autumn's colorful wisdom",
            "the contemplative season",
            "fall's gentle transformation",
            "the harvest of memories",
            "autumn's golden serenity"
        };
        return autumn_msgs[rand() % 5];
    }
}

char* get_day_phase(void) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    int hour = tm->tm_hour;
    
    if (hour < 6) return "in the quiet depths of night";
    else if (hour < 9) return "as dawn paints the sky";
    else if (hour < 12) return "in the morning's fresh promise";
    else if (hour < 15) return "under the midday sun";
    else if (hour < 18) return "in the afternoon's gentle flow";
    else if (hour < 21) return "as twilight approaches";
    else return "in the evening's peaceful embrace";
}

void generate_welcome_message(char *message, size_t size, const config_t *config) {
    char *username = get_username(config);
    char *time_greeting = get_time_greeting();
    char *season_info = get_season_info();
    char *day_phase = get_day_phase();
    
    // Seed random number generator
    srand((unsigned int)time(NULL));
    
    // Array of personalized message templates
    const char *templates[] = {
        "Welcome back, %s! Ready to capture thoughts %s?",
        "Hello %s! How is %s treating you today?",
        "Greetings, %s! Time to chronicle this moment %s.",
        "Hey there, %s! Let's make some memories %s.",
        "%s, welcome to your sanctuary %s!",
        "Good to see you again, %s! The day awaits your words %s.",
        "Hello %s! Ready to weave today's story %s?",
        "Welcome, %s! Your thoughts have a home here %s.",
        "Ah, %s returns! Time to document life %s.",
        "Greetings, dear %s! Let's capture the essence of %s.",
        "Welcome home, %s! Your digital diary awaits %s.",
        "Hello %s! Ready to paint today with words %s?",
        "%s, your storyteller's haven beckons %s!",
        "Welcome back to your realm of reflection, %s! %s seems perfect for writing.",
        "Greetings, %s! The blank page yearns for your wisdom %s."
    };
    
    // Special occasion messages
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    int month = tm->tm_mon + 1;
    int day = tm->tm_mday;
    int hour = tm->tm_hour;
    int wday = tm->tm_wday;
    
    // Special cases
    if (month == 1 && day == 1) {
        snprintf(message, size, "🎉 Happy New Year, %s! What better way to start than with fresh thoughts? %s", username, day_phase);
    }
    else if (month == 12 && day == 25) {
        snprintf(message, size, "🎄 Merry Christmas, %s! Even holidays deserve thoughtful documentation %s.", username, day_phase);
    }
    else if (month == 10 && day == 31) {
        snprintf(message, size, "🎃 Happy Halloween, %s! Time to record some spooky thoughts %s.", username, day_phase);
    }
    else if (wday == 1 && hour < 10) { // Monday morning
        snprintf(message, size, "Monday warrior %s! Let's conquer this week one entry at a time %s.", username, day_phase);
    }
    else if (wday == 5 && hour > 17) { // Friday evening
        snprintf(message, size, "TGIF, %s! Time to reflect on the week's journey %s.", username, day_phase);
    }
    else if (hour < 4) { // Late night
        snprintf(message, size, "Night owl %s! Those midnight thoughts are often the most profound %s.", username, day_phase);
    }
    else if (hour > 22) { // Late evening
        snprintf(message, size, "Evening contemplator %s! Perfect time for reflection %s.", username, day_phase);
    }
    else {
        // Regular personalized messages
        int template_choice = rand() % (sizeof(templates) / sizeof(templates[0]));
        int context_choice = rand() % 3;
        
        const char *context;
        switch (context_choice) {
            case 0: context = time_greeting; break;
            case 1: context = season_info; break;
            default: context = day_phase; break;
        }
        
        snprintf(message, size, templates[template_choice], username, context);
    }
}

void show_personalized_welcome(const config_t *config) {
    char welcome_message[512];
    
    // Check if personalization is enabled
    if (!config->enable_personalization) {
        clear();
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        const char *simple_msg = "Welcome to Ciary!";
        mvprintw(rows / 2, (cols - strlen(simple_msg)) / 2, "%s", simple_msg);
        const char *prompt = "Press any key to continue...";
        mvprintw(rows / 2 + 2, (cols - strlen(prompt)) / 2, "%s", prompt);
        refresh();
        getch();
        return;
    }
    
    generate_welcome_message(welcome_message, sizeof(welcome_message), config);
    
    clear();
    
    // Get screen dimensions
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    
    int title_height = 0;
    int start_row;
    
    // Show ASCII art if enabled
    if (config->show_ascii_art) {
        const char *title[] = {
            "  _____ _                   ",
            " / ____(_)                  ",
            "| |     _  __ _ _ __ _   _   ",
            "| |    | |/ _` | '__| | | |  ",
            "| |____| | (_| | |  | |_| |  ",
            " \\_____|_|\\__,_|_|   \\__, | ",
            "                     __/ |  ",
            "                    |___/   "
        };
        
        title_height = sizeof(title) / sizeof(title[0]);
        start_row = (rows - title_height - 6) / 2;
        
        // Draw title
        attron(A_BOLD);
        for (int i = 0; i < title_height; i++) {
            mvprintw(start_row + i, (cols - strlen(title[i])) / 2, "%s", title[i]);
        }
        attroff(A_BOLD);
    } else {
        // Simple text title
        start_row = (rows - 4) / 2;
        const char *simple_title = "Ciary";
        attron(A_BOLD);
        mvprintw(start_row, (cols - strlen(simple_title)) / 2, "%s", simple_title);
        attroff(A_BOLD);
        title_height = 1;
    }
    
    // Draw welcome message (wrapped if necessary)
    int message_row = start_row + title_height + 2;
    int message_len = strlen(welcome_message);
    int max_width = cols - 4;
    
    if (message_len <= max_width) {
        mvprintw(message_row, (cols - message_len) / 2, "%s", welcome_message);
    } else {
        // Simple word wrap
        char *word_start = welcome_message;
        char *line_start = welcome_message;
        int line_len = 0;
        
        while (*word_start) {
            char *word_end = word_start;
            while (*word_end && *word_end != ' ') word_end++;
            
            int word_len = word_end - word_start;
            
            if (line_len + word_len > max_width && line_len > 0) {
                // Print current line
                char line[512];
                int len = word_start - line_start;
                strncpy(line, line_start, len);
                line[len] = '\0';
                mvprintw(message_row++, (cols - strlen(line)) / 2, "%s", line);
                
                line_start = word_start;
                line_len = 0;
            }
            
            line_len += word_len + 1;
            word_start = word_end;
            if (*word_start == ' ') word_start++;
        }
        
        // Print final line
        if (line_len > 0) {
            mvprintw(message_row, (cols - strlen(line_start)) / 2, "%s", line_start);
        }
    }
    
    // Draw continuation prompt
    const char *prompt = "Press any key to begin your journaling journey...";
    mvprintw(rows - 3, (cols - strlen(prompt)) / 2, "%s", prompt);
    
    // Add some subtle decoration
    mvprintw(message_row + 2, cols / 2 - 10, "～ ～ ～ ～ ～ ～ ～ ～ ～ ～");
    
    refresh();
    getch();
}

void show_personalized_goodbye(const config_t *config) {
    if (!config->enable_personalization) {
        printf("Thank you for using Ciary!\n");
        return;
    }
    
    char *username = get_username(config);
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    int hour = tm->tm_hour;
    
    const char *farewell_msgs[] = {
        "Until next time, %s! Your thoughts are safe with Ciary.",
        "Farewell, %s! May your words echo through time.",
        "See you soon, %s! The pages await your return.",
        "Goodbye for now, %s! Your story continues...",
        "Take care, %s! Your diary will be here when you return.",
        "Au revoir, %s! Keep those thoughts flowing.",
        "Until we meet again, %s! Happy journaling!",
        "Farewell, dear %s! Your chronicles are treasured here."
    };
    
    const char *time_specific_msgs[] = {
        "Sweet dreams, %s! Let tonight's rest inspire tomorrow's words.",     // Night
        "Have a wonderful morning, %s! May the day bring inspiration.",      // Morning  
        "Enjoy your afternoon, %s! Don't forget to capture those moments.",  // Afternoon
        "Have a peaceful evening, %s! Perfect time for reflection.",         // Evening
    };
    
    srand((unsigned int)time(NULL) + 42); // Different seed than welcome
    
    char goodbye_msg[256];
    
    if (hour >= 22 || hour < 6) {
        snprintf(goodbye_msg, sizeof(goodbye_msg), time_specific_msgs[0], username);
    } else if (hour < 12) {
        snprintf(goodbye_msg, sizeof(goodbye_msg), time_specific_msgs[1], username);
    } else if (hour < 18) {
        snprintf(goodbye_msg, sizeof(goodbye_msg), time_specific_msgs[2], username);
    } else {
        snprintf(goodbye_msg, sizeof(goodbye_msg), time_specific_msgs[3], username);
    }
    
    // Sometimes use a general farewell instead
    if (rand() % 3 == 0) {
        int choice = rand() % (sizeof(farewell_msgs) / sizeof(farewell_msgs[0]));
        snprintf(goodbye_msg, sizeof(goodbye_msg), farewell_msgs[choice], username);
    }
    
    printf("%s\n", goodbye_msg);
}