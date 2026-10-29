@echo off
setlocal

set PORT=%1
if "%PORT%"=="" set PORT=8080

echo ============================================================
echo   Starting Disaster Rescue ^& Resource Allocation System
echo ============================================================
echo Server:    http://localhost:%PORT%
echo Dashboard: http://localhost:%PORT%/index.html
echo Port:      %PORT%
echo.
echo Press CTRL+C to terminate the server.
echo ============================================================

if not exist "disaster_server.exe" (
    echo Binary disaster_server.exe not found! Compiling first...
    call build.bat
)

start "" http://localhost:%PORT%
disaster_server.exe %PORT%
endlocal
