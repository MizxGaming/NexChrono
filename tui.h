#ifndef TUI_H
#define TUI_H

#include <string>
#include <string_view>
#include <ncurses.h> // For WINDOW type

// Function declarations from tui.cpp
void draw_layout(WINDOW*& header_win, WINDOW*& status_win, WINDOW*& menu_win, WINDOW*& timer_win);
std::string get_input(std::string_view prompt);
void run_tui();
void draw_large_string_horizontally(WINDOW* win, int start_y, int start_x, const std::string& text, int scale_x, int scale_y);

#endif // TUI_H
