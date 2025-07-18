#include "tui.h"
#include "main.h"
#include <ncurses.h>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm> // For std::max
#include <cstring>   // For strlen

// This function now only creates/recreates the windows. No refreshing.
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

void draw_large_number(WINDOW* win, int start_y, int start_x, const std::string& number_str) {
    int digit_width = large_digits[0][0].length();
    int digit_height = large_digits[0].size();
    int current_x = start_x;

    for (char c : number_str) {
        if (c >= '0' && c <= '9') {
            int digit_index = c - '0';
            for (int i = 0; i < digit_height; ++i) {
                mvwprintw(win, start_y + i, current_x, large_digits[digit_index][i].c_str());
            }
            current_x += digit_width + 1; // +1 for spacing between digits
        } else if (c == ':') {
            for (int i = 0; i < digit_height; ++i) {
                mvwprintw(win, start_y + i, current_x, large_digits[10][i].c_str()); // Index 10 for colon
            }
            current_x += large_digits[10][0].length() + 1; // +1 for spacing
        }
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
    wrefresh(input_win); // Refresh only this small, temporary window

    char input_str[50];
    echo();
    curs_set(1);
    wgetstr(input_win, input_str);
    curs_set(0);
    noecho();

    delwin(input_win);

    return std::string(input_str);
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

    // Initial drawing of timer_win border and title
    box(timer_win, 0, 0);
    mvwprintw(timer_win, 0, 2, "[ Timer ]");

    while (true) {
        // --- EFFICIENT REDRAW SECTION ---
        // Erase window contents, not the whole screen
        werase(header_win);
        werase(status_win);
        werase(menu_win);
        // werase(timer_win); // Handled more granularly below

        // Re-draw the layout borders and titles
        box(header_win, 0, 0);
        mvwprintw(header_win, 2, (getmaxx(header_win) - 9) / 2, "NexChrono");

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

        // Refresh the virtual screen for each window
        wnoutrefresh(header_win);
        wnoutrefresh(status_win);
        wnoutrefresh(menu_win);
        // wnoutrefresh(timer_win); // Handled after content update

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

        // Clear only the content area of the timer window
        int timer_win_h, timer_win_w;
        getmaxyx(timer_win, timer_win_h, timer_win_w);
        for (int i = 1; i < timer_win_h - 1; ++i) { // Clear rows between borders
            mvwprintw(timer_win, i, 1, std::string(timer_win_w - 2, ' ').c_str());
        }

        if (running_task) {
            long long total_elapsed = running_task->elapsed_seconds;
            total_elapsed += std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - running_task->start_time).count();

            long long hours = total_elapsed / 3600;
            long long minutes = (total_elapsed % 3600) / 60;
            long long seconds = total_elapsed % 60;

            char time_str[100];
            sprintf(time_str, "%02lld:%02lld:%02lld", hours, minutes, seconds);

            int digit_height = large_digits[0].size();
            int digit_width = large_digits[0][0].length();
            int colon_width = large_digits[10][0].length();

            // Calculate total width of the large time string (HH:MM:SS)
            // 6 digits * digit_width + 2 colons * colon_width + 7 spaces (between each char)
            int total_large_time_width = (6 * digit_width) + (2 * colon_width) + 7; // Approximation

            int start_y = (timer_win_h - digit_height) / 2;
            int start_x = (timer_win_w - total_large_time_width) / 2;

            draw_large_number(timer_win, start_y, start_x, time_str);
        } else {
            mvwprintw(timer_win, timer_win_h / 2, (timer_win_w - strlen("No task running")) / 2, "No task running");
        }
        wnoutrefresh(timer_win);
        doupdate();

        if (int ch = getch(); ch != ERR) {
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
                case '\n': { // Enter key
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
                    } else if (current_selection == 1) { // Stop Task
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
                    } else if (current_selection == 2) { // Clear Data
                        if (get_input("Are you sure? (y/n): ") == "y") {
                            if (get_input("Type 'confirm' to delete all data: ") == "confirm") {
                                clear_data();
                            }
                        }
                    } else if (current_selection == 3) { // Exit
                        delwin(header_win);
                        delwin(status_win);
                        delwin(menu_win);
                        delwin(timer_win);
                        endwin();
                        return;
                    }
                    break;
                }
            }
        }
    }
}