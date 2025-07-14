#include "main.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>

// Function to clear the terminal screen
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// The main function for the TUI
void run_tui() {
    std::string command;

    while (true) {
        clear_screen();
        std::cout << "--- Time Tracker TUI ---" << std::endl;
        std::cout << "--------------------------" << std::endl;
        show_status();
        std::cout << "--------------------------" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  start <task_name> - Start or resume a task" << std::endl;
        std::cout << "  stop <task_name>  - Stop a running task" << std::endl;
        std::cout << "  clear             - Clear all task data" << std::endl;
        std::cout << "  exit              - Exit the application" << std::endl;
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
                // After start_task finishes (user presses 's'), wait for user to press Enter to continue
                std::cout << "\nPress Enter to return to the menu...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
             std::cout << "\nPress Enter to return to the menu...";
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
             // Since we are not in the main input loop, we need to consume the newline
             if (std::cin.peek() == '\n') {
                std::cin.ignore();
             }
        } else if (cmd == "status") {
            // Status is already shown in the main loop
            continue;
        } else if (cmd == "clear") {
            clear_data();
             std::cout << "\nPress Enter to return to the menu...";
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
             if (std::cin.peek() == '\n') {
                std::cin.ignore();
             }
        } else if (cmd == "exit") {
            break;
        } else {
            if (!cmd.empty()) {
                std::cout << "Unknown command." << std::endl;
                 std::cout << "\nPress Enter to return to the menu...";
                 std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                 if (std::cin.peek() == '\n') {
                    std::cin.ignore();
                 }
            }
        }
    }
}
