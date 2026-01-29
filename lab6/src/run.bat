@echo off
echo ================================
echo Temperature Monitor GUI
echo ================================
echo.

cd /d "%~dp0\build"

REM Check for executable in different locations
if exist "Release\temperature_monitor.exe" (
    set "EXE_PATH=Release\temperature_monitor.exe"
) else if exist "temperature_monitor.exe" (
    set "EXE_PATH=temperature_monitor.exe"
) else if exist "Debug\temperature_monitor.exe" (
    set "EXE_PATH=Debug\temperature_monitor.exe"
) else (
    echo Error: Executable not found!
    echo Please run build.bat first
    echo.
    pause
    exit /b 1
)

echo Starting Temperature Monitor...
echo.
echo Make sure the server from lab5 is running on http://localhost:8080
echo.
echo If the server is not running, open another terminal and run:
echo   cd ..\lab5\src
echo   run.bat
echo.

REM Check if Qt DLLs are in PATH
where Qt6Core.dll >nul 2>nul
if %errorlevel% neq 0 (
    where Qt5Core.dll >nul 2>nul
    if %errorlevel% neq 0 (
        echo Warning: Qt DLLs not found in PATH!
        echo If the application fails to start, add Qt bin directory to PATH:
        echo   set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%%PATH%%
        echo.
    )
)

start "" "%EXE_PATH%"

echo Application started!
echo.
pause
