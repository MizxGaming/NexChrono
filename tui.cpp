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
     "#     #",
     " ##### "},
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
     " ##### ",
     "      #",
     "      #",
     "      #"},
    // 5
    {" ##### ",
     "#     #",
     "#      ",
     " ##### ",
     "      #",
     "#     #",
     " ##### "},
    // 6
    {" ##### ",
     "#      ",
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
     " ##### ",
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
R"(    __      _     ____     __     __     ____   __    __   ______       ____        __      _     ____   )",
R"(   /  \    / )   / __ \   (_ \   / _)   / ___) (  \  /  ) (   __ \     / __ \      /  \    / )   / __ \  )",
R"(  / /\ \  / /   / /  \ \    \ \_/ /    / /      \ (__) /   ) (__) )   / /  \ \    / /\ \  / /   / /  \ \ )",
R"(  ) ) ) ) ) )  ( ()  () )    \   /    ( (        ) __ (   (    __/   ( ()  () )   ) ) ) ) ) )  ( ()  () ))",
R"( ( ( ( ( ( (   ( ()  () )    / _ \    ( (       ( (  ) )   ) \ \  _  ( ()  () )  ( ( ( ( ( (   ( ()  () ))",
R"( / /  \ \/ /    \ \__/ /   _/ / \ \_   \ \___    ) )( (   ( ( \ \_))  \ \__/ /   / /  \ \/ /    \ \__/ / )",
R"((_/    \__/      \____/   (__/   \__)   \____)  /_/  \_\   )_) \__/    \____/   (_/    \__/      \____/  )"
};


void draw_large_string_horizontally(WINDOW* win, int start_y, int start_x, const std::string& text, int scale_x, int scale_y) {
    int base_digit_height = large_digits[0].size();
    int base_digit_width = large_digits[0][0].length();
    int current_x = start_x;

    for (char c : text) {
        const std::vector<std::string>* digit_art;
        if (c >= '0' && c <= '9') {
            digit_art = &large_digits[c - '0'];
        } else if (c == ':') {
            digit_art = &large_digits[10];
        } else {
            continue;
        }

        for (int r = 0; r < base_digit_height; ++r) {
            std::string line = digit_art->at(r);
            std::string scaled_line;
            for (char ch : line) {
                scaled_line += std::string(scale_x, ch);
            }
            for (int sy = 0; sy < scale_y; ++sy) {
                if (start_y + (r * scale_y) + sy < getmaxy(win) && current_x < getmaxx(win)) {
                    mvwprintw(win, start_y + (r * scale_y) + sy, current_x, scaled_line.c_str());
                }
            }
        }
        current_x += (base_digit_width + 1) * scale_x;
    }
}

void draw_layout(WINDOW*& header_win, WINDOW*& status_win, WINDOW*& menu_win, WINDOW*& timer_win) {
    int term_y, term_x;
    getmaxyx(stdscr, term_y, term_x);

    int main_w = term_x * 2 / 3; // 2/3 of the screen for main content
    int timer_w = term_x / 3;    // 1/3 for the timer

    // Header window
    int header_h = 10;
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

std::string get_input(std::string_view prompt) {
    int term_y, term_x;
    getmaxyx(stdscr, term_y, term_x);

    int input_win_h = 3;
    int input_win_w = std::max(40, (int)prompt.length() + 15); // +15 for input and border
    WINDOW* input_win = newwin(input_win_h, input_win_w, (term_y - input_win_h) / 2, (term_x - input_win_w) / 2);
    box(input_win, 0, 0);
    keypad(input_win, TRUE);
    curs_set(1);
    noecho();

    std::string input_str;
    int ch;

    while (true) {
        // Clear the line, print prompt and current input
        wmove(input_win, 1, 1);
        wclrtoeol(input_win);
        box(input_win, 0, 0); // Redraw box in case it was cleared
        mvwprintw(input_win, 1, 2, "%s%s", prompt.data(), input_str.c_str());
        wrefresh(input_win);

        ch = wgetch(input_win);

        if (ch == 10) { // Enter
            break;
        } else if (ch == 27) { // Escape
            input_str.clear();
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (!input_str.empty()) {
                input_str.pop_back();
            }
        } else if (isprint(ch)) {
            // Ensure input doesn't exceed the visible area within the box
            if (prompt.length() + input_str.length() < (size_t)input_win_w - 4) {
                input_str += (char)ch;
            }
        }
    }

    curs_set(0);
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

        int timer_win_h, timer_win_w;
        getmaxyx(timer_win, timer_win_h, timer_win_w);

        if (running_task) {
            long long total_elapsed = running_task->elapsed_seconds;
            total_elapsed += std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - running_task->start_time).count();

            long long hours = total_elapsed / 3600;
            long long minutes = (total_elapsed % 3600) / 60;
            long long seconds = total_elapsed % 60;

            char hours_str[3], mins_str[3], secs_str[3];
            sprintf(hours_str, "%02lld", hours);
            sprintf(mins_str, "%02lld", minutes);
            sprintf(secs_str, "%02lld", seconds);

            int base_digit_height = 7;
            int base_digit_width = 7;
            int row_spacing = 2; // Vertical space between HH, MM, SS
            int padding = 2;     // Padding from the window borders

            int component_base_width = (base_digit_width * 2) + 1;
            int component_base_height = base_digit_height;

            int available_width = timer_win_w - 2 * padding;
            int available_height = timer_win_h - 2 * padding;

            int total_content_base_height = (component_base_height * 3) + (row_spacing * 2);

            int scale_x = (available_width > 0) ? available_width / component_base_width : 0;
            int scale_y = (available_height > 0) ? available_height / total_content_base_height : 0;

            if (scale_x > 0 && scale_y > 0) {
                int scale = std::min(scale_x, scale_y);

                int scaled_comp_width = component_base_width * scale;
                int scaled_comp_height = component_base_height * scale;
                int total_content_height = (scaled_comp_height * 3) + (row_spacing * 2);

                int start_x = (timer_win_w - scaled_comp_width) / 2;
                int start_y = (timer_win_h - total_content_height) / 2;

                draw_large_string_horizontally(timer_win, start_y, start_x, hours_str, scale, scale);
                draw_large_string_horizontally(timer_win, start_y + scaled_comp_height + row_spacing, start_x, mins_str, scale, scale);
                draw_large_string_horizontally(timer_win, start_y + 2 * (scaled_comp_height + row_spacing), start_x, secs_str, scale, scale);
            } else {
                char time_str[100];
                sprintf(time_str, "%02lld:%02lld:%02lld", hours, minutes, seconds);
                mvwprintw(timer_win, timer_win_h / 2, (timer_win_w - strlen(time_str)) / 2, time_str);
            }
        } else {
            const char* msg = "No task running";
            mvwprintw(timer_win, timer_win_h / 2, (timer_win_w - strlen(msg)) / 2, msg);
        }
        wrefresh(timer_win);
        wrefresh(timer_win); // Refresh the timer window specifically to show the update

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
