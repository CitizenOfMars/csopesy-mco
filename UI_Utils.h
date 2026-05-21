#pragma once
#include <iostream>
#include <cstdlib>

namespace UI_Utils {
    inline void printHeader() {
        std::cout << R"(
  _____  _____    ____   _    _  _____        _  _   
 / ____||  __ \  / __ \ | |  | ||  __ \      | || |  
| |  __ | |__) || |  | || |  | || |__) |     | || |_ 
| | |_ ||  _  / | |  | || |  | ||  ___/      |__   _|
| |__| || | \ \ | |__| || |__| || |             | |  
 \_____||_|  \_\ \____/  \____/ |_|             |_|  

Welcome to Group 4 Emulator!
Type 'exit' to quit, 'clear' to clear the screen
)";
}

    inline void clearScreen() {
        #ifdef _WIN32
            std::system("cls");
        #else
            std::system("clear");
        #endif
    }
}