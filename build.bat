@echo off
REM Build script for keylogger

REM Create build directory
if not exist build mkdir build
cd build

REM Generate CMake files
cmake ..

REM Build the project
cmake --build .

echo.
echo Build complete!
echo The executable is ready in the build directory.
echo.

pause 