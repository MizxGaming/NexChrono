#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void stop_task(const std::string& task_name);

const std::string FILE_PATH = "/home/mizx/Documents/YPT-Linux/timetracker.csv";

struct Task {
    std::string name;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    long long elapsed_seconds;
    bool running;
    std::string date;
};

std::vector<Task> read_tasks() {
    std::vector<Task> tasks;
    std::ifstream file(FILE_PATH);
    if (!file.is_open()) {
        return tasks;
    }

    std::string line;
    // Skip header
    std::getline(file, line);

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
            long long start_count;
            std::stringstream(start_str) >> start_count;
            task.start_time = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(start_count));
        }

        if (end_str.empty() || end_str == "0") {
            task.running = true;
        } else {
            long long end_count;
            std::stringstream(end_str) >> end_count;
            if (end_count != 0) {
                 task.end_time = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(end_count));
            }
        }
        
        if (!elapsed_str.empty()) {
            task.elapsed_seconds = std::stoll(elapsed_str);
        } else {
            task.elapsed_seconds = 0;
        }

        task.date = date_str;
        tasks.push_back(task);
    }

    file.close();
    return tasks;
}

void write_tasks(const std::vector<Task>& tasks) {
    std::ofstream file(FILE_PATH);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    file << "task,start_time,end_time,elapsed_time,date\n";

    for (const auto& task : tasks) {
        file << task.name << ",";
        file << task.start_time.time_since_epoch().count() << ",";
        if (!task.running) {
            file << task.end_time.time_since_epoch().count() << ",";
            file << task.elapsed_seconds << "," << task.date;
        } else {
            file << "0," << task.elapsed_seconds << "," << task.date;
        }
        file << "\n";
    }

    file.close();
}

void start_task(const std::string& task_name) {
    std::vector<Task> tasks = read_tasks();
    auto it = std::find_if(tasks.begin(), tasks.end(), [&](const Task& task){
        return task.name == task_name;
    });

    if (it != tasks.end()) {
        // Task exists
        if (it->running) {
            std::cout << "Task '" << task_name << "' is already running." << std::endl;
            return;
        }
        // It's stopped, so restart it
        it->start_time = std::chrono::system_clock::now();
        it->running = true;
    } else {
        // Task does not exist, create a new one
        Task new_task;
        new_task.name = task_name;
        new_task.start_time = std::chrono::system_clock::now();
        new_task.running = true;
        new_task.elapsed_seconds = 0;
        tasks.push_back(new_task);
        it = tasks.end() - 1;
    }

    // Get current date
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    it->date = ss.str();

    write_tasks(tasks);
    std::cout << "Started task '" << task_name << "'." << std::endl;

    // UI for showing elapsed time and stopping the task
    while (true) {
        auto current_time = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - it->start_time).count();
        long long total_elapsed = it->elapsed_seconds + elapsed;

        long long hours = total_elapsed / 3600;
        long long minutes = (total_elapsed % 3600) / 60;
        long long seconds = total_elapsed % 60;

        std::cout << "\r" << "Task: " << it->name << " | Elapsed time: " 
                  << std::setfill('0') << std::setw(2) << hours << ":"
                  << std::setfill('0') << std::setw(2) << minutes << ":"
                  << std::setfill('0') << std::setw(2) << seconds 
                  << " | Press 's' to stop..." << std::flush;

        // Check for user input to stop the task
        // This is a non-blocking way to check for input
        fd_set set;
        struct timeval timeout;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
        if (rv > 0 && FD_ISSET(STDIN_FILENO, &set)) {
            char c;
            read(STDIN_FILENO, &c, 1);
            if (c == 's') {
                stop_task(task_name);
                break;
            }
        }

        // Update running status from file to allow external stop commands
        std::vector<Task> current_tasks = read_tasks();
        auto current_it = std::find_if(current_tasks.begin(), current_tasks.end(), [&](const Task& task){
            return task.name == task_name;
        });
        if (current_it == current_tasks.end() || !current_it->running) {
            std::cout << "\nTask stopped externally." << std::endl;
            break;
        }
    }
}

void stop_task(const std::string& task_name) {
    std::vector<Task> tasks = read_tasks();
    bool found = false;

    for (auto& task : tasks) {
        if (task.name == task_name && task.running) {
            task.running = false;
            task.end_time = std::chrono::system_clock::now();
            task.elapsed_seconds += std::chrono::duration_cast<std::chrono::seconds>(task.end_time - task.start_time).count();
            found = true;
            break;
        }
    }

    if (found) {
        write_tasks(tasks);
        std::cout << "\nStopped task '" << task_name << "'." << std::endl;
    } else {
        std::cout << "\nTask '" << task_name << "' is not running." << std::endl;
    }
}

void show_status() {
    std::vector<Task> tasks = read_tasks();

    if (tasks.empty()) {
        std::cout << "No tasks to show." << std::endl;
        return;
    }

    std::cout << std::left << std::setw(20) << "Task" << std::setw(20) << "Status" << std::setw(20) << "Date" << "Elapsed Time" << std::endl;
    std::cout << "---------------------------------------------------------------------" << std::endl;

    for (const auto& task : tasks) {
        long long total_elapsed = task.elapsed_seconds;
        if (task.running) {
            total_elapsed += std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - task.start_time).count();
        }

        long long hours = total_elapsed / 3600;
        long long minutes = (total_elapsed % 3600) / 60;
        long long seconds = total_elapsed % 60;

        std::cout << std::left << std::setw(20) << task.name
                  << std::setw(20) << (task.running ? "Running" : "Stopped")
                  << std::setw(20) << task.date;
        std::cout << std::setfill('0') << std::setw(2) << hours << ":"
                  << std::setfill('0') << std::setw(2) << minutes << ":"
                  << std::setfill('0') << std::setw(2) << seconds << std::endl;
        std::cout << std::setfill(' ');
    }
}

void clear_data() {
    std::string confirmation;
    std::cout << "Are you sure you want to clear all data? This action cannot be undone. (y/n): ";
    std::getline(std::cin, confirmation);

    if (confirmation == "y") {
        std::cout << "Please type 'confirm' to proceed: ";
        std::getline(std::cin, confirmation);
        if (confirmation == "confirm") {
            std::vector<Task> empty_tasks;
            write_tasks(empty_tasks);
            std::cout << "All data has been cleared." << std::endl;
        } else {
            std::cout << "Clear operation cancelled." << std::endl;
        }
    } else {
        std::cout << "Clear operation cancelled." << std::endl;
    }
}


int main() {
    std::string command;
    std::cout << "Time Tracker" << std::endl;
    std::cout << "Commands: start <task>, stop <task>, status, clear, exit" << std::endl;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);

        std::stringstream ss(command);
        std::string cmd;
        ss >> cmd;

        if (cmd == "start") {
            std::string task_name;
            ss >> task_name;
            if (!task_name.empty()) {
                start_task(task_name);
            } else {
                std::cout << "Please provide a task name." << std::endl;
            }
        } else if (cmd == "stop") {
            std::string task_name;
            ss >> task_name;
            if (!task_name.empty()) {
                stop_task(task_name);
            } else {
                std::cout << "Please provide a task name." << std::endl;
            }
        }
        else if (cmd == "status") {
            show_status();
        } else if (cmd == "clear") {
            clear_data();
        } else if (cmd == "exit") {
            break;
        } else {
            if(!cmd.empty()){
                std::cout << "Unknown command." << std::endl;
            }
        }
    }

    return 0;
}
