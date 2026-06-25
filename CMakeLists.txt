cmake_minimum_required(VERSION 3.10)
project(csopesy VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/ConsoleManager.cpp
    src/Scheduler.cpp
    src/ConfigParser.cpp
)

# Create executable
add_executable(csopesy ${SOURCES})

# Link threading library (required for std::thread, std::mutex, etc.)
target_link_libraries(csopesy PRIVATE Threads::Threads)
find_package(Threads REQUIRED)

# Compiler-specific flags
if(MSVC)
    # Visual Studio
    target_compile_options(csopesy PRIVATE /W4)
else()
    # GCC/Clang
    target_compile_options(csopesy PRIVATE -Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_options(csopesy PRIVATE -g)
    endif()
endif()

# Output directory
set_target_properties(csopesy PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Print build info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Project: csopesy")