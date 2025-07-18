#include "tui.h"
#include "main.h"
#include <ncurses.h>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm> // For std::max
#include <cctype>   // For isprint
#include <cstring>   // For strlen

// ASCII art for large digits
const std::vector<std::string> large_digits[] = {
    // 0
    {" ##### ",
     "#     #",
     "#     #",
     "#     #",
     "#     #",
     "#     #",
     " ##### "},
    // 1
    {"   #   ",
     "  ##   ",
     " # #   ",
     "   #   ",
     "   #   ",
     "   #   ",
     " ##### "},
    // 2
    {" ##### ",
     "#     #",
     "      #",
     " ##### ",
     "#      ",
     "#      ",
     "#######"},
    // 3
    {" ##### ",
     "#     #",
     "      #",
     " ##### ",
     "      #",
     "#     #",
     " ##### "},
    // 4
    {"#     #",
     "#     #",
     "#     #",
     " #######",
     "      #",
     "      #",
     "      #"},
    // 5
    {"#######",
     "#      ",
     "#      ",
     " ##### ",
     "      #",
     "#     #",
     " ##### "},
    // 6
    {" ##### ",
     "#     #",
     "#      ",
     " ##### ",
     "#     #",
     "#     #",
     " ##### "},
    // 7
    {"#######",
     "      #",
     "      #",
     "     # ",
     "    #  ",
     "   #   ",
     "  #    "},
    // 8
    {" ##### ",
     "#     #",
     "#     #",
     " ##### ",
     "#     #",
     "#     #",
     " ##### "},
    // 9
    {" ##### ",
     "#     #",
     "#     #",
     " #######",
     "      #",
     "      #",
     " ##### "},
    // :
    {"       ",
     "   #   ",
     "       ",
     "       ",
     "   #   ",
     "       ",
     "       "}
};

// ASCII art for "NoxChrono"
const std::vector<std::string> noxchrono_art = {
    R"( _   _             _                      _                )",
    R"(| \ | | ___   ___ | | __ ___   ___   ___ | |__   ___  ___ )",
    R"(|  \| |/ _ \ / _ \| |/ // _ \ / _ \ / _ \| '_ \ / _ \/ __|)",
    R"(| |\  | (_) | (_) |   <| (_) | (_) | (_) | |_) | (_) \__ \)",
    R"(|_| \_|\___/ \___/|_|\_\\___/ \___/ \___/|_.__/ \___/|___/)"
};


void draw_large_number(WINDOW* win, int start_y, int start_x, const std::string& number_str, int scale_x, int scale_y) {
    int base_digit_height = large_digits[0].size(); // 7
    int base_digit_width = large_digits[0][0].length(); // 7

    int current_y = start_y; // Start drawing from this y-coordinate

    for (char c : number_str) {
        const std::vector<std::string>* digit_art;
        if (c >= '0' && c <= '9') {
            digit_art = &large_digits[c - '0'];
        } else if (c == ':') {
            digit_art = &large_digits[10];
        } else {
            continue;
        }

        // Create a rotated version of the current character's ASCII art
        // New dimensions: height = original_width, width = original_height
        int original_height = digit_art->size();
        int original_width = digit_art->at(0).length(); // Assuming all lines have same length for a digit

        std::vector<std::string> rotated_art(original_width, std::string(original_height, ' '));

        for (int r = 0; r < original_height; ++r) {
            for (int c_orig = 0; c_orig < original_width; ++c_orig) {
                if (digit_art->at(r).length() > c_orig && digit_art->at(r)[c_orig] != ' ') {
                    // Clockwise rotation: (new_row, new_col) = (original_col, original_height - 1 - original_row)
                    rotated_art[c_orig][original_height - 1 - r] = digit_art->at(r)[c_orig];
                }
            }
        }

        // Now print the rotated and scaled character
        int rotated_char_height = original_width; // New height is original width
        int rotated_char_width = original_height; // New width is original height

        for (int r_rot = 0; r_rot < rotated_char_height; ++r_rot) {
            std::string line = rotated_art[r_rot];
            std::string scaled_line = "";
            for (char ch : line) {
                for (int sx = 0; sx < scale_x; ++sx) {
                    scaled_line += ch;
                }
            }
            for (int sy = 0; sy < scale_y; ++sy) {
                mvwprintw(win, current_y + (r_rot * scale_y) + sy, start_x, scaled_line.c_str());
            }
        }
        current_y += (rotated_char_height + 1) * scale_y; // Move down for next character, +1 for spacing
    }
}

void draw_layout(WINDOW*& header_win, WINDOW*& status_win, WINDOW*& menu_win, WINDOW*& timer_win) {
    int term_y, term_x;
    getmaxyx(stdscr, term_y, term_x);

    int main_w = term_x * 2 / 3; // 2/3 of the screen for main content
    int timer_w = term_x / 3;    // 1/3 for the timer

    // Header window
    int header_h = 5;
    if (header_win) delwin(header_win);
    header_win = newwin(header_h, main_w, 0, 0);

    // Status window
    int status_h = term_y - header_h - 8;
    if (status_win) delwin(status_win);
    status_win = newwin(status_h, main_w, header_h, 0);

    // Menu window
    int menu_h = 8;
    if (menu_win) delwin(menu_win);
    menu_win = newwin(menu_h, main_w, term_y - menu_h, 0);

    // Timer window
    if (timer_win) delwin(timer_win);
    timer_win = newwin(term_y, timer_w, 0, main_w);
}

// This function now only gets input and does not touch the main screen.
std::string get_input(std::string_view prompt) {
    int term_y, term_x;
    getmaxyx(stdscr, term_y, term_x);

    int input_win_h = 3;
    int input_win_w = 40;
    WINDOW* input_win = newwin(input_win_h, input_win_w, (term_y - input_win_h) / 2, (term_x - input_win_w) / 2);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 2, prompt.data());
    wrefresh(input_win);

    std::string input_str;
    int ch;
    curs_set(1); // Show cursor
    noecho();    // Disable echoing of characters

    while (true) {
        ch = wgetch(input_win);

        if (ch == 27) { // Escape key
            input_str.clear(); // Clear any typed input
            break;
        } else if (ch == 10) { // Enter key
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127) { // Backspace or Delete
            if (!input_str.empty()) {
                input_str.pop_back();
                mvwdelch(input_win, 1, 2 + prompt.length() + input_str.length()); // Erase character from screen
                wrefresh(input_win);
            }
        } else if (isprint(ch)) { // Printable character
            if (input_str.length() < 49) { // Limit input length
                input_str += (char)ch;
                mvwaddch(input_win, 1, 2 + prompt.length() + input_str.length() - 1, ch);
                wrefresh(input_win);
            }
        }
    }

    curs_set(0); // Hide cursor
    delwin(input_win);

    return input_str;
}

void run_tui() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000); // Refresh every second

    WINDOW *header_win = nullptr, *status_win = nullptr, *menu_win = nullptr, *timer_win = nullptr;
    draw_layout(header_win, status_win, menu_win, timer_win);

    const std::vector<std::string_view> menu_items = {"Start Task", "Stop Task", "Clear Data", "Exit"};
    int current_selection = 0;

    while (true) {
        // --- EFFICIENT REDRAW SECTION ---
        // Erase window contents, not the whole screen
        werase(header_win);
        werase(status_win);
        werase(menu_win);
        werase(timer_win); // Now clear the whole timer window

        // Re-draw the layout borders and titles
        box(header_win, 0, 0);
        // Draw NoxChrono art
        for (size_t i = 0; i < noxchrono_art.size(); ++i) {
            mvwprintw(header_win, i + 1, (getmaxx(header_win) - noxchrono_art[i].length()) / 2, noxchrono_art[i].c_str());
        }

        box(status_win, 0, 0);
        mvwprintw(status_win, 0, 2, "[ Tasks ]");
        show_status(status_win); // Ask show_status to fill its window

        box(menu_win, 0, 0);
        mvwprintw(menu_win, 0, 2, "[ Menu ]");
        for (size_t i = 0; i < menu_items.size(); ++i) {
            if (i == current_selection) wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, i + 2, 4, menu_items[i].data());
            if (i == current_selection) wattroff(menu_win, A_REVERSE);
        }

        box(timer_win, 0, 0); // Draw border and title for timer window
        mvwprintw(timer_win, 0, 2, "[ Timer ]");

        // Refresh the virtual screen for each window
        wnoutrefresh(header_win);
        wnoutrefresh(status_win);
        wnoutrefresh(menu_win);
        wnoutrefresh(timer_win); // Refresh the whole timer window

        // Update the physical screen once
        doupdate();
        // --- END OF REDRAW SECTION ---

        // Timer display logic
        std::vector<Task> tasks = read_tasks();
        Task* running_task = nullptr;
        for (auto& task : tasks) {
            if (task.running) {
                running_task = &task;
                break;
            }
        }

        // No need to clear content area explicitly, werase(timer_win) handles it
        int timer_win_h, timer_win_w;
        getmaxyx(timer_win, timer_win_h, timer_win_w);

        if (running_task) {
            long long total_elapsed = running_task->elapsed_seconds;
            total_elapsed += std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - running_task->start_time).count();

            long long hours = total_elapsed / 3600;
            long long minutes = (total_elapsed % 3600) / 60;
            long long seconds = total_elapsed % 60;

            char time_str[100];
            sprintf(time_str, "%02lld:%02lld:%02lld", hours, minutes, seconds);

            // Calculate total base dimensions of the large time string (HH:MM:SS) after rotation
            // Each character is now (base_digit_width x base_digit_height) rotated to (base_digit_height x base_digit_width)
            // Total height = 6 chars * base_digit_height + 2 colons * base_digit_height + 7 spaces (between each char)
            // Total width = base_digit_width (max of all chars)
            int rotated_char_base_height = large_digits[0][0].length(); // Original width becomes new height
            int rotated_char_base_width = large_digits[0].size(); // Original height becomes new width

            // For HH:MM:SS, there are 8 characters (6 digits, 2 colons)
            // Total height will be sum of heights of 8 characters + 7 spaces between them
            int total_base_rotated_height = (rotated_char_base_height * 8) + 7; // 8 chars * 7 rows/char + 7 spaces
            int total_base_rotated_width = rotated_char_base_width; // Max width of a rotated char

            int scale_x = std::max(1, (timer_win_w - 2) / total_base_rotated_width);
            int scale_y = std::max(1, (timer_win_h - 2) / total_base_rotated_height);

            // No maximum scale, let it fill the window as much as possible
            // Ensure at least scale 1
            if (scale_x == 0) scale_x = 1;
            if (scale_y == 0) scale_y = 1;

            int scaled_total_rotated_height = total_base_rotated_height * scale_y;
            int scaled_total_rotated_width = total_base_rotated_width * scale_x;

            int start_y = (timer_win_h - scaled_total_rotated_height) / 2;
            int start_x = (timer_win_w - scaled_total_rotated_width) / 2;

            draw_large_number(timer_win, start_y, start_x, time_str, scale_x, scale_y);
        } else {
            mvwprintw(timer_win, timer_win_h / 2, (timer_win_w - strlen("No task running")) / 2, "No task running");
        }

        int ch = getch();
        if (ch != ERR) {
            if (ch == KEY_RESIZE) {
                endwin();
                refresh();
                clear();
                draw_layout(header_win, status_win, menu_win, timer_win);
                continue;
            }

            switch (ch) {
                case KEY_UP:
                    current_selection = (current_selection - 1 + menu_items.size()) % menu_items.size();
                    break;
                case KEY_DOWN:
                    current_selection = (current_selection + 1) % menu_items.size();
                    break;
                case '\n': // Enter key
                case 's': // Start Task shortcut
                    if (ch == 's') current_selection = 0;
                    if (current_selection == 0) { // Start Task
                        if (auto task_name = get_input("Start Task Name: "); !task_name.empty()) {
                            if (!start_task(task_name)) {
                                int win_h = 7;
                                int win_w = 62;
                                WINDOW* warning_win = newwin(win_h, win_w, (LINES - win_h) / 2, (COLS - win_w) / 2);
                                box(warning_win, 0, 0);

                                const char* line1 = "A task is already running.";
                                const char* line2 = "Maybe you would like to stop the previous task first?";
                                const char* button = "< OK >";

                                mvwprintw(warning_win, 2, (win_w - strlen(line1)) / 2, line1);
                                mvwprintw(warning_win, 3, (win_w - strlen(line2)) / 2, line2);
                                
                                wattron(warning_win, A_REVERSE);
                                mvwprintw(warning_win, 5, (win_w - strlen(button)) / 2, button);
                                wattroff(warning_win, A_REVERSE);
                                
                                wrefresh(warning_win);
                                while(getch() != '\n');
                                delwin(warning_win);
                            }
                        }
                    } 
                    // Fallthrough for other shortcuts
                case 'S': // Stop Task shortcut
                    if (ch == 'S') current_selection = 1;
                    if (current_selection == 1) { // Stop Task
                        std::vector<Task> current_tasks = read_tasks();
                        Task* running_task_to_stop = nullptr;
                        for (auto& task : current_tasks) {
                            if (task.running) {
                                running_task_to_stop = &task;
                                break;
                            }
                        }

                        if (running_task_to_stop) {
                            std::string prompt = "Stop task '" + running_task_to_stop->name + "'? (y/n): ";
                            if (get_input(prompt) == "y") {
                                stop_task(running_task_to_stop->name);
                            }
                        } else {
                            // Display a message that no task is running
                            int win_h = 5;
                            int win_w = 40;
                            WINDOW* no_task_win = newwin(win_h, win_w, (LINES - win_h) / 2, (COLS - win_w) / 2);
                            box(no_task_win, 0, 0);
                            const char* msg = "No task is currently running.";
                            const char* button = "< OK >";
                            mvwprintw(no_task_win, 1, (win_w - strlen(msg)) / 2, msg);
                            wattron(no_task_win, A_REVERSE);
                            mvwprintw(no_task_win, 3, (win_w - strlen(button)) / 2, button);
                            wattroff(no_task_win, A_REVERSE);
                            wrefresh(no_task_win);
                            while(getch() != '\n');
                            delwin(no_task_win);
                        }
                    }
                    // Fallthrough for other shortcuts
                case 'X': // Clear Data shortcut
                    if (ch == 'X') current_selection = 2;
                    if (current_selection == 2) { // Clear Data
                        if (get_input("Are you sure? (y/n): ") == "y") {
                            if (get_input("Type 'confirm' to delete all data: ") == "confirm") {
                                clear_data();
                            }
                        }
                    }
                    // Fallthrough for other shortcuts
                case 'q': // Exit shortcut
                    if (ch == 'q') current_selection = 3;
                    if (current_selection == 3) { // Exit
                        delwin(header_win);
                        delwin(status_win);
                        delwin(menu_win);
                        delwin(timer_win);
                        endwin();
                        return;
                    }
                    break;
                case 27: // Escape key
                    // Handled by get_input, or does nothing if no input field is open
                    break;
            }
        }
    }
}
