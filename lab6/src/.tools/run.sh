#!/bin/bash
# Unified build and run script for lab6 (Linux/macOS)
set -e

MODE=${1:-run}
SCRIPT_DIR="$(dirname "$0")"
cd "$SCRIPT_DIR"

if [ "$MODE" = "build" ]; then
    echo "================================"
    echo "Building Temperature Monitor GUI"
    echo "================================"
    if ! command -v qmake &> /dev/null && ! command -v qmake6 &> /dev/null; then
        echo "Error: Qt not found!"
        echo "Please install Qt."
        exit 1
    fi
    if [ ! -d "../build" ]; then
        mkdir ../build
    fi
    cd ../build
    echo "Running CMake..."
    cmake ..
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
    exit $?
fi

if [ "$MODE" = "run" ]; then
    echo "================================"
    echo "Temperature Monitor GUI"
    echo "================================"
    cd ../build
    if [ ! -f "temperature_monitor" ]; then
        echo "Error: Executable not found!"
        echo "Please run ./run.sh build first"
        exit 1
    fi
    echo "Starting Temperature Monitor..."
    echo "Make sure the server from lab5 is running on http://localhost:8080"
    ./temperature_monitor
    exit $?
fi

echo "Usage: $0 [build|run]"
exit 1
