#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <chrono>

struct Task {
    std::string name;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    long long elapsed_seconds;
    bool running;
    std::string date;
};

std::vector<Task> read_tasks();
void write_tasks(const std::vector<Task>& tasks);
void start_task(const std::string& task_name);
void stop_task(const std::string& task_name);
void show_status();
void clear_data();

#endif // MAIN_H
