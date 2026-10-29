@echo off
setlocal
cd /d "%~dp0"

echo ============================================================
echo   Starting Disaster Rescue System Web Server (C++17)
echo ============================================================
echo.
echo Launching server at http://localhost:8080 ...
start http://localhost:8080
disaster_server.exe
