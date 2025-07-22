#include "../include/ciary.h"
#include <dirent.h>
#include <sys/wait.h>

#ifdef HAVE_LIBHARU
#include <hpdf.h>
#include <math.h>
#endif

// Parse date from filename (YYYY-MM-DD.md format)
bool parse_date_from_filename(const char* filename, date_t* date) {
    if (!filename || !date) return false;
    
    // Check if filename ends with .md
    size_t len = strlen(filename);
    if (len < 13 || strcmp(filename + len - 3, ".md") != 0) {
        return false;
    }
    
    // Parse YYYY-MM-DD format
    int year, month, day;
    if (sscanf(filename, "%d-%d-%d.md", &year, &month, &day) != 3) {
        return false;
    }
    
    // Validate date values
    if (year < 1900 || year > 3000 || 
        month < 1 || month > 12 ||
        day < 1 || day > 31) {
        return false;
    }
    
    date->year = year;
    date->month = month;
    date->day = day;
    
    return true;
}

// Calculate date range based on preset
void calculate_date_range(date_range_preset_t preset, date_t current_date, date_t *start, date_t *end) {
    *start = current_date;
    *end = current_date;
    
    switch (preset) {
        case DATE_RANGE_LAST_7_DAYS:
            date_add_days(start, -6);  // 7 days including today
            break;
            
        case DATE_RANGE_THIS_MONTH:
            start->day = 1;
            end->day = days_in_month(current_date.month, current_date.year);
            break;
            
        case DATE_RANGE_THIS_YEAR:
            start->month = 1;
            start->day = 1;
            end->month = 12;
            end->day = 31;
            break;
            
        case DATE_RANGE_ALL:
            // Set to a very wide range - we'll scan the journal directory
            start->year = 1900;
            start->month = 1;
            start->day = 1;
            end->year = 2100;
            end->month = 12;
            end->day = 31;
            break;
            
        case DATE_RANGE_CUSTOM:
            // Keep the provided dates as-is
            break;
    }
}

// Show progress bar
void show_progress_bar(const char *message, int current, int total) {
    int bar_width = 40;
    float progress = (float)current / total;
    int pos = bar_width * progress;
    
    // Clear the status bar area
    move(LINES - 1, 0);
    clrtoeol();
    
    // Show message and progress bar
    mvprintw(LINES - 1, 2, "%s [", message);
    
    for (int i = 0; i < bar_width; i++) {
        if (i < pos) {
            addch('=');
        } else if (i == pos) {
            addch('>');
        } else {
            addch(' ');
        }
    }
    
    printw("] %d/%d (%d%%)", current, total, (int)(progress * 100));
    refresh();
}

// Display export dialog and get user preferences
int show_export_dialog(app_state_t *state, export_options_t *options) {
    char input[256];
    int choice;
    
    // Initialize defaults
    options->format = EXPORT_FORMAT_HTML;
    options->include_empty_days = 0;
    snprintf(options->output_path, MAX_PATH_SIZE, "%s", state->config.journal_directory);
    
    // Clear screen and show export dialog
    clear();
    
    // Title
    mvprintw(2, (COLS - 20) / 2, "=== EXPORT ENTRIES ===");
    
    // Date range selection
    mvprintw(4, 4, "Select date range:");
    mvprintw(5, 6, "1. All entries");
    mvprintw(6, 6, "2. Last 7 days");
    mvprintw(7, 6, "3. This month");
    mvprintw(8, 6, "4. This year");
    mvprintw(9, 6, "5. Custom range");
    
    mvprintw(11, 4, "Choice [1-5]: ");
    refresh();
    
    // Get date range choice
    if (getnstr(input, sizeof(input) - 1) == ERR) return 0;
    choice = atoi(input);
    
    date_range_preset_t preset;
    switch (choice) {
        case 1: preset = DATE_RANGE_ALL; break;
        case 2: preset = DATE_RANGE_LAST_7_DAYS; break;
        case 3: preset = DATE_RANGE_THIS_MONTH; break;
        case 4: preset = DATE_RANGE_THIS_YEAR; break;
        case 5: preset = DATE_RANGE_CUSTOM; break;
        default: return 0;  // Cancel
    }
    
    if (preset == DATE_RANGE_CUSTOM) {
        // Get custom start date
        mvprintw(13, 4, "Start date (YYYY-MM-DD): ");
        refresh();
        if (getnstr(input, sizeof(input) - 1) == ERR) return 0;
        if (sscanf(input, "%d-%d-%d", &options->start_date.year, 
                   &options->start_date.month, &options->start_date.day) != 3) {
            return 0;
        }
        
        // Get custom end date
        mvprintw(14, 4, "End date (YYYY-MM-DD): ");
        refresh();
        if (getnstr(input, sizeof(input) - 1) == ERR) return 0;
        if (sscanf(input, "%d-%d-%d", &options->end_date.year, 
                   &options->end_date.month, &options->end_date.day) != 3) {
            return 0;
        }
    } else {
        calculate_date_range(preset, state->current_date, &options->start_date, &options->end_date);
    }
    
    // Format selection with dynamic availability
    mvprintw(16, 4, "Export format:");
    mvprintw(17, 6, "1. HTML (always available)");
    
    // Check PDF availability
    int pdf_available = 0;
    char pdf_note[256] = "";
    
#ifdef HAVE_LIBHARU
    pdf_available = 1;
    strcpy(pdf_note, "2. PDF (native libHaru)");
#else
    // Check for external PDF tools
    if (system("which wkhtmltopdf > /dev/null 2>&1") == 0) {
        pdf_available = 1;
        strcpy(pdf_note, "2. PDF (via wkhtmltopdf)");
    } else if (system("which weasyprint > /dev/null 2>&1") == 0) {
        pdf_available = 1;
        strcpy(pdf_note, "2. PDF (via weasyprint)");
    } else {
        strcpy(pdf_note, "2. PDF (unavailable - install wkhtmltopdf/weasyprint or libHaru)");
    }
#endif
    
    mvprintw(18, 6, "%s", pdf_note);
    mvprintw(19, 6, "3. Markdown (always available)");
    
    if (pdf_available) {
        mvprintw(21, 4, "Format [1-3]: ");
    } else {
        mvprintw(21, 4, "Format [1,3] (PDF unavailable): ");
    }
    refresh();
    
    if (getnstr(input, sizeof(input) - 1) == ERR) return 0;
    choice = atoi(input);
    
    switch (choice) {
        case 1: 
            options->format = EXPORT_FORMAT_HTML; 
            break;
        case 2: 
            if (pdf_available) {
                options->format = EXPORT_FORMAT_PDF;
            } else {
                mvprintw(23, 4, "PDF export not available. Press any key to continue...");
                refresh();
                getch();
                return 0;
            }
            break;
        case 3: 
            options->format = EXPORT_FORMAT_MARKDOWN; 
            break;
        default: 
            return 0;
    }
    
    // Output location
    mvprintw(25, 4, "Output directory [%s]: ", options->output_path);
    refresh();
    
    if (getnstr(input, sizeof(input) - 1) != ERR && strlen(input) > 0) {
        snprintf(options->output_path, MAX_PATH_SIZE, "%s", input);
    }
    
    // Confirmation
    mvprintw(27, 4, "Export %d-%02d-%02d to %d-%02d-%02d in %s format? (y/N): ",
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day,
             (options->format == EXPORT_FORMAT_HTML) ? "HTML" :
             (options->format == EXPORT_FORMAT_PDF) ? "PDF" : "Markdown");
    refresh();
    
    if (getnstr(input, sizeof(input) - 1) == ERR) return 0;
    return (input[0] == 'y' || input[0] == 'Y');
}

// Helper structure for sorting entries by date
typedef struct {
    char *filename;
    date_t date;
} entry_file_t;

// Comparison function for qsort to sort entries chronologically
int compare_entry_files(const void *a, const void *b) {
    const entry_file_t *entry_a = (const entry_file_t *)a;
    const entry_file_t *entry_b = (const entry_file_t *)b;
    
    return date_compare(entry_a->date, entry_b->date);
}

// Collect all entry files in the specified date range (sorted chronologically)
int collect_entries_in_range(const export_options_t *options, const config_t *config, 
                           char ***entry_files, int *file_count) {
    DIR *dir;
    struct dirent *entry;
    entry_file_t entries[1000];  // Reasonable limit
    int count = 0;
    
    dir = opendir(config->journal_directory);
    if (!dir) {
        return 0;
    }
    
    // Collect all valid entries with their dates
    while ((entry = readdir(dir)) != NULL && count < 1000) {
        // Check if it's a .md file with date format YYYY-MM-DD.md
        if (strstr(entry->d_name, ".md") && strlen(entry->d_name) == 13) {
            date_t file_date;
            if (sscanf(entry->d_name, "%d-%d-%d.md", 
                      &file_date.year, &file_date.month, &file_date.day) == 3) {
                
                // Check if file date is in range
                if (date_compare(file_date, options->start_date) >= 0 &&
                    date_compare(file_date, options->end_date) <= 0) {
                    
                    // Allocate memory for filename
                    entries[count].filename = malloc(MAX_PATH_SIZE);
                    if (entries[count].filename) {
                        int result = snprintf(entries[count].filename, MAX_PATH_SIZE, "%s/%s", 
                                config->journal_directory, entry->d_name);
                        if (result > 0 && result < MAX_PATH_SIZE) {
                            entries[count].date = file_date;
                            count++;
                        } else {
                            free(entries[count].filename);
                        }
                    }
                }
            }
        }
    }
    
    closedir(dir);
    
    if (count == 0) {
        *entry_files = NULL;
        *file_count = 0;
        return 1;
    }
    
    // Sort entries chronologically
    qsort(entries, count, sizeof(entry_file_t), compare_entry_files);
    
    // Allocate array for return
    *entry_files = malloc(count * sizeof(char*));
    if (!*entry_files) {
        // Free allocated memory on failure
        for (int i = 0; i < count; i++) {
            free(entries[i].filename);
        }
        return 0;
    }
    
    // Copy sorted file pointers
    for (int i = 0; i < count; i++) {
        (*entry_files)[i] = entries[i].filename;
    }
    
    *file_count = count;
    return 1;
}

// Export entries to HTML format
int export_to_html(const export_options_t *options, const config_t *config, 
                  char **entry_files, int file_count) {
    (void)config;  // Suppress unused parameter warning
    char output_file[MAX_PATH_SIZE];
    FILE *output;
    FILE *input;
    char line[MAX_LINE_SIZE];
    char title[256];
    
    // Create output filename
    int result = snprintf(output_file, MAX_PATH_SIZE, "%s/ciary_export_%d-%02d-%02d_to_%d-%02d-%02d.html",
             options->output_path,
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    
    if (result >= MAX_PATH_SIZE) {
        return 0; // Path too long
    }
    
    output = fopen(output_file, "w");
    if (!output) {
        return 0;
    }
    
    // Generate title
    snprintf(title, sizeof(title), "Ciary Export: %d-%02d-%02d to %d-%02d-%02d",
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    
    // Write HTML header
    fprintf(output, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(output, "<meta charset=\"UTF-8\">\n");
    fprintf(output, "<title>%s</title>\n", title);
    fprintf(output, "<style>\n");
    fprintf(output, "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; ");
    fprintf(output, "max-width: 800px; margin: 40px auto; padding: 20px; line-height: 1.6; }\n");
    fprintf(output, "h1 { color: #333; border-bottom: 3px solid #007acc; padding-bottom: 10px; }\n");
    fprintf(output, "h2 { color: #555; margin-top: 30px; }\n");
    fprintf(output, "h3 { color: #777; margin-top: 20px; }\n");
    fprintf(output, ".entry-date { background: #f8f9fa; padding: 15px; border-left: 4px solid #007acc; margin: 20px 0; }\n");
    fprintf(output, ".entry-time { background: #fff; border-left: 3px solid #ddd; padding: 10px; margin: 10px 0; }\n");
    fprintf(output, "pre { background: #f8f9fa; padding: 10px; border-radius: 4px; overflow-x: auto; }\n");
    fprintf(output, "code { background: #f1f1f1; padding: 2px 4px; border-radius: 3px; }\n");
    fprintf(output, ".footer { margin-top: 40px; text-align: center; color: #666; font-size: 0.9em; }\n");
    fprintf(output, "</style>\n");
    fprintf(output, "</head>\n<body>\n");
    fprintf(output, "<h1>%s</h1>\n", title);
    fprintf(output, "<p>Generated by Ciary on %s</p>\n", __DATE__);
    
    // Process each entry file
    for (int i = 0; i < file_count; i++) {
        show_progress_bar("Exporting to HTML", i + 1, file_count);
        
        input = fopen(entry_files[i], "r");
        if (!input) continue;
        
        // Extract date from filename for section header
        char *filename = strrchr(entry_files[i], '/');
        if (filename) filename++;
        else filename = entry_files[i];
        
        fprintf(output, "<div class=\"entry-date\">\n");
        fprintf(output, "<h2>%s</h2>\n", filename);
        
        // Process file content
        int in_time_section = 0;
        while (fgets(line, sizeof(line), input)) {
            // Remove newline
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
            }
            
            // Check for time headers (## HH:MM:SS)
            if (strncmp(line, "## ", 3) == 0) {
                if (in_time_section) {
                    fprintf(output, "</div>\n");
                }
                fprintf(output, "<div class=\"entry-time\">\n");
                fprintf(output, "<h3>%s</h3>\n", line + 3);
                in_time_section = 1;
            }
            // Check for date headers (# YYYY-MM-DD)
            else if (strncmp(line, "# ", 2) == 0) {
                // Skip date headers as we already have them
                continue;
            }
            // Regular content
            else if (strlen(line) > 0) {
                // Simple markdown to HTML conversion
                if (strncmp(line, "```", 3) == 0) {
                    fprintf(output, "<pre><code>");
                } else {
                    // Escape HTML characters and convert basic markdown
                    fprintf(output, "<p>");
                    for (char *c = line; *c; c++) {
                        switch (*c) {
                            case '<': fprintf(output, "&lt;"); break;
                            case '>': fprintf(output, "&gt;"); break;
                            case '&': fprintf(output, "&amp;"); break;
                            default: fputc(*c, output); break;
                        }
                    }
                    fprintf(output, "</p>\n");
                }
            }
        }
        
        if (in_time_section) {
            fprintf(output, "</div>\n");
        }
        fprintf(output, "</div>\n");
        
        fclose(input);
    }
    
    // Write HTML footer
    fprintf(output, "<div class=\"footer\">\n");
    fprintf(output, "<p>Exported from Ciary - A minimalistic TUI diary application</p>\n");
    fprintf(output, "</div>\n");
    fprintf(output, "</body>\n</html>\n");
    
    fclose(output);
    return 1;
}

#ifdef HAVE_LIBHARU
// LibHaru error handler
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    (void)user_data;  // Suppress unused parameter warning
    fprintf(stderr, "ERROR: libHaru error 0x%04X, detail 0x%04X\n", (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
}

// Export entries to PDF using native libHaru
int export_to_pdf_native(const export_options_t *options, const config_t *config, 
                         char **entry_files, int file_count) {
    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Font title_font, date_font, time_font, content_font;
    char output_file[MAX_PATH_SIZE];
    FILE *input;
    char line[MAX_LINE_SIZE];
    float y_position;
    const float margin = 50.0;
    const float page_width = 595.0;   // A4 width in points
    const float page_height = 842.0;  // A4 height in points
    const float content_width = page_width - (margin * 2);
    
    // Create output filename
    int result = snprintf(output_file, MAX_PATH_SIZE, "%s/ciary_export_%d-%02d-%02d_to_%d-%02d-%02d.pdf",
             options->output_path,
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    
    if (result >= MAX_PATH_SIZE) {
        return 0; // Path too long
    }
    
    // Create PDF document
    pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        return 0;
    }
    
    // Set compression mode
    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    
    // Create first page
    page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    
    // Load fonts
    title_font = HPDF_GetFont(pdf, "Helvetica-Bold", NULL);
    date_font = HPDF_GetFont(pdf, "Helvetica-Bold", NULL);
    time_font = HPDF_GetFont(pdf, "Helvetica-BoldOblique", NULL);
    content_font = HPDF_GetFont(pdf, "Helvetica", NULL);
    
    // Initial Y position
    y_position = page_height - margin;
    
    // Title
    HPDF_Page_SetFontAndSize(page, title_font, 20);
    HPDF_Page_BeginText(page);
    char title[256];
    snprintf(title, sizeof(title), "Ciary Export: %d-%02d-%02d to %d-%02d-%02d",
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    
    float title_width = HPDF_Page_TextWidth(page, title);
    HPDF_Page_TextOut(page, (page_width - title_width) / 2, y_position, title);
    HPDF_Page_EndText(page);
    
    y_position -= 40;
    
    // Subtitle
    HPDF_Page_SetFontAndSize(page, content_font, 10);
    HPDF_Page_BeginText(page);
    char subtitle[] = "Generated by Ciary - A minimalistic TUI diary application";
    float subtitle_width = HPDF_Page_TextWidth(page, subtitle);
    HPDF_Page_TextOut(page, (page_width - subtitle_width) / 2, y_position, subtitle);
    HPDF_Page_EndText(page);
    
    y_position -= 30;
    
    // Draw separator line
    HPDF_Page_SetLineWidth(page, 1);
    HPDF_Page_MoveTo(page, margin, y_position);
    HPDF_Page_LineTo(page, page_width - margin, y_position);
    HPDF_Page_Stroke(page);
    
    y_position -= 20;
    
    // Process each entry file
    for (int i = 0; i < file_count; i++) {
        show_progress_bar("Generating native PDF", i + 1, file_count);
        
        input = fopen(entry_files[i], "r");
        if (!input) continue;
        
        // Extract date from filename
        char *filename = strrchr(entry_files[i], '/');
        if (filename) filename++;
        else filename = entry_files[i];
        
        // Check if we need a new page
        if (y_position < margin + 100) {
            page = HPDF_AddPage(pdf);
            HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
            y_position = page_height - margin;
        }
        
        // Date header
        HPDF_Page_SetFontAndSize(page, date_font, 16);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, margin, y_position, filename);
        HPDF_Page_EndText(page);
        y_position -= 25;
        
        // Process file content
        int in_time_section = 0;
        while (fgets(line, sizeof(line), input) && y_position > margin + 20) {
            // Remove newline
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                len--;
            }
            
            // Skip empty lines
            if (len == 0) {
                y_position -= 10;
                continue;
            }
            
            // Check if we need a new page
            if (y_position < margin + 50) {
                page = HPDF_AddPage(pdf);
                HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
                y_position = page_height - margin;
            }
            
            // Check for time headers (## HH:MM:SS)
            if (strncmp(line, "## ", 3) == 0) {
                y_position -= 10; // Add space before time section
                HPDF_Page_SetFontAndSize(page, time_font, 12);
                HPDF_Page_BeginText(page);
                HPDF_Page_TextOut(page, margin + 20, y_position, line + 3);
                HPDF_Page_EndText(page);
                y_position -= 20;
                in_time_section = 1;
            }
            // Skip date headers (# YYYY-MM-DD)
            else if (strncmp(line, "# ", 2) == 0) {
                continue;
            }
            // Regular content
            else if (strlen(line) > 0) {
                HPDF_Page_SetFontAndSize(page, content_font, 10);
                
                // Simple word wrapping
                char *word = strtok(line, " ");
                char current_line[512] = "";
                float line_width = 0;
                
                while (word) {
                    char test_line[512];
                    if (strlen(current_line) > 0) {
                        snprintf(test_line, sizeof(test_line), "%s %s", current_line, word);
                    } else {
                        snprintf(test_line, sizeof(test_line), "%s", word);
                    }
                    
                    HPDF_Page_SetFontAndSize(page, content_font, 10);
                    float test_width = HPDF_Page_TextWidth(page, test_line);
                    
                    if (test_width <= content_width - 40) {
                        strcpy(current_line, test_line);
                    } else {
                        // Print current line and start new one
                        if (strlen(current_line) > 0) {
                            HPDF_Page_BeginText(page);
                            HPDF_Page_TextOut(page, margin + 40, y_position, current_line);
                            HPDF_Page_EndText(page);
                            y_position -= 15;
                        }
                        strcpy(current_line, word);
                        
                        // Check if we need a new page
                        if (y_position < margin + 30) {
                            page = HPDF_AddPage(pdf);
                            HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
                            y_position = page_height - margin;
                        }
                    }
                    word = strtok(NULL, " ");
                }
                
                // Print remaining line
                if (strlen(current_line) > 0) {
                    HPDF_Page_BeginText(page);
                    HPDF_Page_TextOut(page, margin + 40, y_position, current_line);
                    HPDF_Page_EndText(page);
                    y_position -= 15;
                }
            }
        }
        
        y_position -= 20; // Space between entries
        fclose(input);
    }
    
    // Save PDF
    HPDF_STATUS status = HPDF_SaveToFile(pdf, output_file);
    HPDF_Free(pdf);
    
    return (status == HPDF_OK);
}
#endif

// Export entries to PDF with intelligent fallback
int export_to_pdf(const export_options_t *options, const config_t *config, 
                 char **entry_files, int file_count) {
#ifdef HAVE_LIBHARU
    // Try native libHaru first
    int result = export_to_pdf_native(options, config, entry_files, file_count);
    if (result) {
        return 1;
    }
    // If native fails, fall through to external tools
#endif
    
    // Fallback to external tools
    char html_file[MAX_PATH_SIZE];
    char pdf_file[MAX_PATH_SIZE];
    char command[MAX_PATH_SIZE * 3];  // Space for command + two file paths
    int ext_result;
    
    // First create HTML file
    if (!export_to_html(options, config, entry_files, file_count)) {
        return 0;
    }
    
    // Generate filenames
    int html_result = snprintf(html_file, MAX_PATH_SIZE, "%s/ciary_export_%d-%02d-%02d_to_%d-%02d-%02d.html",
             options->output_path,
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    
    int pdf_result = snprintf(pdf_file, MAX_PATH_SIZE, "%s/ciary_export_%d-%02d-%02d_to_%d-%02d-%02d.pdf",
             options->output_path,
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    
    if (html_result >= MAX_PATH_SIZE || pdf_result >= MAX_PATH_SIZE) {
        return 0; // Path too long
    }
    
    // Try wkhtmltopdf first, then weasyprint as fallback
    if (system("which wkhtmltopdf > /dev/null 2>&1") == 0) {
        int cmd_result = snprintf(command, sizeof(command), "wkhtmltopdf '%s' '%s' 2>/dev/null", html_file, pdf_file);
        if (cmd_result >= (int)sizeof(command)) {
            return 0; // Command too long
        }
        show_progress_bar("Converting HTML to PDF (wkhtmltopdf)", 1, 1);
    } else if (system("which weasyprint > /dev/null 2>&1") == 0) {
        int cmd_result = snprintf(command, sizeof(command), "weasyprint '%s' '%s' 2>/dev/null", html_file, pdf_file);
        if (cmd_result >= (int)sizeof(command)) {
            return 0; // Command too long
        }
        show_progress_bar("Converting HTML to PDF (weasyprint)", 1, 1);
    } else {
        // No PDF converter available - keep HTML file as fallback
        return 0;
    }
    
    ext_result = system(command);
    
    // Clean up temporary HTML file only if PDF was created successfully
    if (ext_result == 0) {
        unlink(html_file);
    }
    
    return (ext_result == 0);
}

// Export entries to Markdown format
int export_to_markdown(const export_options_t *options, const config_t *config, 
                      char **entry_files, int file_count) {
    (void)config;  // Suppress unused parameter warning
    char output_file[MAX_PATH_SIZE];
    FILE *output;
    FILE *input;
    char line[MAX_LINE_SIZE];
    
    // Create output filename
    int result = snprintf(output_file, MAX_PATH_SIZE, "%s/ciary_export_%d-%02d-%02d_to_%d-%02d-%02d.md",
             options->output_path,
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    
    if (result >= MAX_PATH_SIZE) {
        return 0; // Path too long
    }
    
    output = fopen(output_file, "w");
    if (!output) {
        return 0;
    }
    
    // Write header
    fprintf(output, "# Ciary Export: %d-%02d-%02d to %d-%02d-%02d\n\n",
             options->start_date.year, options->start_date.month, options->start_date.day,
             options->end_date.year, options->end_date.month, options->end_date.day);
    fprintf(output, "Generated by Ciary on %s\n\n", __DATE__);
    fprintf(output, "---\n\n");
    
    // Process each entry file
    for (int i = 0; i < file_count; i++) {
        show_progress_bar("Exporting to Markdown", i + 1, file_count);
        
        input = fopen(entry_files[i], "r");
        if (!input) continue;
        
        // Copy file content directly (it's already in Markdown format)
        while (fgets(line, sizeof(line), input)) {
            fputs(line, output);
        }
        
        fprintf(output, "\n---\n\n");  // Separator between days
        fclose(input);
    }
    
    fprintf(output, "\n*Exported from Ciary - A minimalistic TUI diary application*\n");
    
    fclose(output);
    return 1;
}

// Main export function
int export_entries(const export_options_t *options, const config_t *config) {
    char **entry_files;
    int file_count;
    int result = 0;
    
    // Collect entry files in range
    if (!collect_entries_in_range(options, config, &entry_files, &file_count)) {
        mvprintw(LINES - 2, 2, "Failed to collect entry files.");
        refresh();
        getch();
        return 0;
    }
    
    if (file_count == 0) {
        mvprintw(LINES - 2, 2, "No entries found in the specified date range.");
        refresh();
        getch();
        
        // Free memory
        free(entry_files);
        return 0;
    }
    
    // Export based on format
    switch (options->format) {
        case EXPORT_FORMAT_HTML:
            result = export_to_html(options, config, entry_files, file_count);
            break;
        case EXPORT_FORMAT_PDF:
            result = export_to_pdf(options, config, entry_files, file_count);
            break;
        case EXPORT_FORMAT_MARKDOWN:
            result = export_to_markdown(options, config, entry_files, file_count);
            break;
    }
    
    // Clean up
    for (int i = 0; i < file_count; i++) {
        free(entry_files[i]);
    }
    free(entry_files);
    
    // Show result
    if (result) {
        const char *format_str = (options->format == EXPORT_FORMAT_HTML) ? "HTML" :
                                (options->format == EXPORT_FORMAT_PDF) ? "PDF" : "Markdown";
        mvprintw(LINES - 2, 2, "Successfully exported %d entries to %s format.", file_count, format_str);
    } else {
        mvprintw(LINES - 2, 2, "Export failed. Check permissions and dependencies.");
    }
    
    refresh();
    getch();
    return result;
}