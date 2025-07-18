#include "tui.h"
#include "main.h"
#include <ncurses.h>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm> // For std::max
#include <cstring>   // For strlen

// This function now only creates/recreates the windows. No refreshing.
void draw_layout(WINDOW*& header_win, WINDOW*& status_win, WINDOW*& menu_win) {
    int term_y, term_x;
    getmaxyx(stdscr, term_y, term_x);

    // Header window
    int header_h = 5;
    if (header_win) delwin(header_win);
    header_win = newwin(header_h, term_x, 0, 0);

    // Status window
    int status_h = term_y - header_h - 8;
    if (status_win) delwin(status_win);
    status_win = newwin(status_h, term_x, header_h, 0);

    // Menu window
    int menu_h = 8;
    if (menu_win) delwin(menu_win);
    menu_win = newwin(menu_h, term_x, term_y - menu_h, 0);
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

    WINDOW *header_win = nullptr, *status_win = nullptr, *menu_win = nullptr;
    draw_layout(header_win, status_win, menu_win);

    const std::vector<std::string_view> menu_items = {"Start Task", "Stop Task", "Clear Data", "Exit"};
    int current_selection = 0;

    while (true) {
        // --- EFFICIENT REDRAW SECTION ---
        // Erase window contents, not the whole screen
        werase(header_win);
        werase(status_win);
        werase(menu_win);

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

        // Update the physical screen once
        doupdate();
        // --- END OF REDRAW SECTION ---

        if (int ch = getch(); ch != ERR) {
            if (ch == KEY_RESIZE) {
                endwin();
                refresh();
                clear();
                draw_layout(header_win, status_win, menu_win);
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
                        if (auto task_name = get_input("Stop Task Name: "); !task_name.empty()) {
                            stop_task(task_name);
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
                        endwin();
                        return;
                    }
                    break;
                }
            }
        }
    }
}