#pragma once
#include <string>

class ConsoleManager {
public:
    void startMainLoop(); 

private:
    bool processCommand(const std::string& command); 
};