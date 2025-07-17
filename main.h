#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <chrono>
#include <ncurses.h> // For WINDOW type

struct Task {
    std::string name;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    long long elapsed_seconds;
    bool running;
    std::string date;
};

// Core data functions
std::vector<Task> read_tasks();
void write_tasks(const std::vector<Task>& tasks);

// Application logic modified for ncurses
void start_task(const std::string& task_name);
void stop_task(const std::string& task_name);
void show_status(WINDOW* win); // Draws status to an ncurses window
void clear_data();             // Performs the data deletion

#endif // MAIN_H
