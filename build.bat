@echo off
setlocal
echo ============================================================
echo   Compiling Disaster Rescue Web Server (C++17)
echo ============================================================
echo Compiler: MinGW-w64 g++
echo Flags:    -std=c++17 -Wall -Wextra -O2 -lws2_32
echo.

g++ -std=c++17 -Wall -Wextra -O2 main.cpp third_party/sqlite3.o -lws2_32 -o disaster_server.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Server compiled successfully: disaster_server.exe
    echo Run "run.bat" to launch the server and open the dashboard.
) else (
    echo [ERROR] Build failed with exit code %ERRORLEVEL%.
)
endlocal
