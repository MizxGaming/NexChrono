

#include "main.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

const fs::path FILE_PATH = "/home/mizx/Documents/YPT-Linux/timetracker.csv";

std::vector<Task> read_tasks() {
    std::vector<Task> tasks;
    if (std::ifstream file(FILE_PATH); file.is_open()) {
        std::string line;
        std::getline(file, line); // Skip header

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string name, start_str, end_str, elapsed_str, date_str;
            std::getline(ss, name, ',');
            std::getline(ss, start_str, ',');
            std::getline(ss, end_str, ',');
            std::getline(ss, elapsed_str, ',');
            std::getline(ss, date_str, ',');

            Task task;
            task.name = name;
            task.running = false;

            if (!start_str.empty()) {
                task.start_time = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(std::stoll(start_str)));
            }

            if (end_str.empty() || end_str == "0") {
                task.running = true;
            } else {
                if (end_str != "0") {
                    task.end_time = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(std::stoll(end_str)));
                }
            }

            task.elapsed_seconds = elapsed_str.empty() ? 0 : std::stoll(elapsed_str);
            task.date = date_str;
            tasks.push_back(task);
        }
    } else {
        if (std::ofstream new_file(FILE_PATH); new_file.is_open()) {
            new_file << "task,start_time,end_time,elapsed_time,date\n";
        }
    }
    return tasks;
}

void write_tasks(const std::vector<Task>& tasks) {
    if (std::ofstream file(FILE_PATH); file.is_open()) {
        file << "task,start_time,end_time,elapsed_time,date\n";
        for (const auto& task : tasks) {
            file << task.name << ",";
            file << task.start_time.time_since_epoch().count() << ",";
            if (!task.running) {
                file << task.end_time.time_since_epoch().count() << ",";
            } else {
                file << "0,";
            }
            file << task.elapsed_seconds << "," << task.date << "\n";
        }
    }
}

bool start_task(std::string_view task_name) {
    auto tasks = read_tasks();

    if (auto it = std::find_if(tasks.begin(), tasks.end(), [](const auto& task) { return task.running; }); it != tasks.end()) {
        return false; // A task is already running
    }

    if (auto it = std::find_if(tasks.begin(), tasks.end(), [&](const auto& task) { return task.name == task_name; }); it != tasks.end()) {
        // Found an existing task
        if (it->running) return false; // Already running, should not happen due to the check above but good for safety
        it->start_time = std::chrono::system_clock::now();
        it->running = true;
        // Update date when restarting
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
        it->date = ss.str();
    } else {
        // Create a new task
        Task new_task;
        new_task.name = task_name;
        new_task.start_time = std::chrono::system_clock::now();
        new_task.running = true;
        new_task.elapsed_seconds = 0;
        
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
        new_task.date = ss.str();
        
        tasks.push_back(new_task);
    }

    write_tasks(tasks);
    return true;
}

void stop_task(std::string_view task_name) {
    auto tasks = read_tasks();
    
    if (auto it = std::find_if(tasks.begin(), tasks.end(), [&](const auto& task) { return task.name == task_name && task.running; }); it != tasks.end()) {
        it->running = false;
        it->end_time = std::chrono::system_clock::now();
        it->elapsed_seconds += std::chrono::duration_cast<std::chrono::seconds>(it->end_time - it->start_time).count();
        write_tasks(tasks);
    }
}

void show_status(WINDOW* win) {
    // Erasing and refreshing is handled by the TUI loop
    std::vector<Task> tasks = read_tasks();

    mvwprintw(win, 1, 2, "%-20s %-10s %-12s %-15s", "Task", "Status", "Date", "Elapsed Time");
    mvwprintw(win, 2, 2, "-----------------------------------------------------------------");

    int row = 3;
    for (const auto& task : tasks) {
        long long total_elapsed = task.elapsed_seconds;
        if (task.running) {
            total_elapsed += std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - task.start_time).count();
        }

        long long hours = total_elapsed / 3600;
        long long minutes = (total_elapsed % 3600) / 60;
        long long seconds = total_elapsed % 60;

        char time_buf[10];
        sprintf(time_buf, "%02lld:%02lld:%02lld", hours, minutes, seconds);

        mvwprintw(win, row, 2, "%-20s %-10s %-12s %-15s",
                  task.name.c_str(),
                  (task.running ? "Running" : "Stopped"),
                  task.date.c_str(),
                  time_buf);
        row++;
    }
}

void clear_data() {
    // This function is now non-interactive.
    // The TUI is responsible for confirmation.
    std::vector<Task> empty_tasks;
    write_tasks(empty_tasks);
}
