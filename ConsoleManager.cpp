#include "ConsoleManager.h"
#include "UI_Utils.h"
#include <iostream>
#include <sstream>

void ConsoleManager::startMainLoop() {
    UI_Utils::printHeader();
    std::string inputLine;
    std::string command;

    while (true) {
        std::cout << "Enter a command: ";
        if (!std::getline(std::cin, inputLine)) {
            break;
        }

        std::istringstream inputStream(inputLine);
        if (!(inputStream >> command)) {
            continue;
        }

        std::string extraToken;
        if (inputStream >> extraToken) {
            std::cout << "command not recognized\n";
            continue;
        }

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