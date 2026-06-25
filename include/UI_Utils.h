#pragma once
#include <iostream>

namespace UI_Utils {
    inline void printHeader() {
        std::cout << R"(
   ____  ____     U  ___ u  ____   U _____ u ____      __   __
U /"___|/ __"| u   \/"_ \/U|  _"\ u\| ___"|// __"| u   \ \ / / 
\| | u <\___ \/    | | | |\| |_) |/ |  _|" <\___ \/     \ V /  
 | |/__ u___) |.-,_| |_| | |  __/   | |___  u___) |    U_|"|_u 
  \____||____/>>\_)-\___/  |_|      |_____| |____/>>     |_|   
 _// \\  )(  (__)    \\    ||>>_    <<   >>  )(  (__).-,//|(_  
(__)(__)(__)        (__)  (__)__)  (__) (__)(__)      \_) (__) 

Welcome to CSOPESY Emulator!
Type 'exit' to quit, 'clear' to clear the screen
)";
}

    inline void clearScreen() {
        std::cout << "\x1B[2J\x1B[H\x1B[3J";
        std::cout.flush();
    }
}