@echo off
REM Compile the C++ code
g++ -std=c++17 -O2 -Wall VersionControl.cpp -o filesystem.exe

REM Check if compilation succeeded
IF %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    exit /b %ERRORLEVEL%
)

REM Run the program (interactive mode)
filesystem.exe