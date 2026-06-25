#include "ConsoleManager.h"
#include "UI_Utils.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

namespace {
    void countdown() {
        for (int i = 0; i < 1; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

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
        std::cout << "clear command recognized. Doing something.\n";
        countdown();
        UI_Utils::clearScreen();
        UI_Utils::printHeader();
        return false;
    } else if (command == "exit") {
        std::cout << "exit command recognized. Doing something.\n";
        countdown();
        return true;
    }

    std::cout << "command not recognized\n";
    return false;
}