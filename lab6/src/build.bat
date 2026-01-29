@echo off
echo ================================
echo Building Temperature Monitor GUI
echo ================================
echo.

cd /d "%~dp0"

REM Check for CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

REM Check for Qt
if not defined Qt6_DIR (
    if not defined Qt5_DIR (
        echo Warning: Qt6_DIR or Qt5_DIR not set!
        echo Please set environment variable or install Qt from https://www.qt.io/
        echo.
        echo Example:
        echo   set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
        echo.
    )
)

REM Create build directory
if not exist "build" mkdir build
cd build

echo.
echo Running CMake...
echo.

REM Try to configure with MinGW first
cmake .. -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo.
    echo MinGW Makefiles failed, trying Visual Studio...
    cmake .. -G "Visual Studio 17 2022" -A x64
    if %errorlevel% neq 0 (
        cmake .. -G "Visual Studio 16 2019" -A x64
        if %errorlevel% neq 0 (
            echo.
            echo Error: CMake configuration failed!
            echo Please ensure Qt is installed and CMake can find it.
            pause
            exit /b 1
        )
    )
)

echo.
echo Building project...
echo.

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo.
    echo Error: Build failed!
    pause
    exit /b 1
)

echo.
echo ================================
echo Build completed successfully!
echo ================================
echo.
echo Executable: .\build\Release\temperature_monitor.exe
echo          or .\build\temperature_monitor.exe
echo.
echo To run:
echo   run.bat
echo.
pause
