@echo off
setlocal
set "PATH=C:\mingw64\bin;%PATH%"

echo Compiling Disaster Rescue ^& Resource System (C++)...
g++ -std=c++17 -Wall -Wextra -static -static-libgcc -static-libstdc++ main.cpp rescue_queue.cpp resources.cpp locations.cpp history.cpp -o disaster_rescue.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Compilation finished! Executable: disaster_rescue.exe
) else (
    echo [ERROR] Compilation failed with error code %ERRORLEVEL%.
)
endlocal
