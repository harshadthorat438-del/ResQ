@echo off
setlocal
set "PATH=C:\mingw64\bin;%PATH%"

echo ============================================================
echo   Compiling Disaster Rescue Web Server (C++17)
echo ============================================================
g++ -std=c++17 -Wall -Wextra -static -static-libgcc -static-libstdc++ server.cpp rescue_queue.cpp resources.cpp locations.cpp history.cpp -lws2_32 -o disaster_server.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Server compiled successfully! Binary: disaster_server.exe
    echo Run "run_server.bat" to start the server and open the web dashboard.
) else (
    echo.
    echo [ERROR] Server compilation failed with error code %ERRORLEVEL%.
)
endlocal
