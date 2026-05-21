#include "ConsoleManager.h"
#include "UI_Utils.h"
#include <iostream>

void ConsoleManager::startMainLoop() {
    UI_Utils::printHeader();
    std::string command;

    while (true) {
        std::cout << "Enter a command: ";
        std::cin >> command;

        if (processCommand(command)) {
            break;
        }
    }
}

bool ConsoleManager::processCommand(const std::string& command) {
    // Recognized commands list: initialize, screen, scheduler-start, scheduler-stop,
    // report-util, clear, exit
    if (command == "initialize") {
        std::cout << "initialize command recognized. Doing something.\n";
        return false;
    } else if (command == "screen") {
        std::cout << "screen command recognized. Doing something.\n";
        return false;
    } else if (command == "scheduler-start") {
        std::cout << "scheduler-start command recognized. Doing something.\n";
        return false;
    } else if (command == "scheduler-stop") {
        std::cout << "scheduler-stop command recognized. Doing something.\n";
        return false;
    } else if (command == "report-util") {
        std::cout << "report-util command recognized. Doing something.\n";
        return false;
    } else if (command == "clear") {
        UI_Utils::clearScreen();
        UI_Utils::printHeader();
        std::cout << "clear command recognized. Doing something.\n";
        return false;
    } else if (command == "exit") {
        std::cout << "exit command recognized.\nExiting.\n";
        return true;
    }

    std::cout << "command not recognized\n";
    return false;
}