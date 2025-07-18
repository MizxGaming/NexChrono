#ifndef TUI_H
#define TUI_H

#include <string>
#include <string_view>
#include <ncurses.h> // For WINDOW type

// Function declarations from tui.cpp
void draw_layout(WINDOW*& header_win, WINDOW*& status_win, WINDOW*& menu_win);
std::string get_input(std::string_view prompt);
void run_tui();

#endif // TUI_H
