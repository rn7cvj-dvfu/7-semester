#!/bin/bash
set -e

PULL=0
REBUILD=0
EXE_NAME="temperature_monitor"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"

for arg in "$@"; do
    case $arg in
        --pull)
            PULL=1
            ;;
        --rebuild)
            REBUILD=1
            ;;
        --exe=*)
            EXE_NAME="${arg#*=}"
            ;;
        build)
            MODE=build
            ;;
        run)
            MODE=run
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            echo "Usage: $0 [--pull] [--rebuild] [--exe=NAME] [build|run]"
            exit 1
            ;;
    esac
done

MODE=${MODE:-run}
cd "$SCRIPT_DIR"

if [ $PULL -eq 1 ]; then
    if ! command -v git &> /dev/null; then
        echo "Error: git not found!" >&2
        exit 1
    fi
    git pull || echo "Warning: git pull failed (maybe no changes)"
fi

if [ $REBUILD -eq 1 ]; then
    echo "================================"
    echo "Building Temperature Monitor GUI"
    echo "================================"
    if ! command -v cmake &> /dev/null; then
        echo "Error: cmake not found!" >&2
        exit 1
    fi
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    echo "Running CMake..."
    cmake ..
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
    cd "$SCRIPT_DIR"
fi

if [ "$MODE" = "run" ]; then
    echo "================================"
    echo "Temperature Monitor GUI"
    echo "================================"
    cd "$BUILD_DIR"
    if [ ! -f "$EXE_NAME" ]; then
        echo "Error: Executable not found!"
        echo "Please run ./run.sh --rebuild first"
        exit 1
    fi
    echo "Starting Temperature Monitor..."
    echo "Make sure the server from lab5 is running on http://localhost:8080"
    ./$EXE_NAME
    exit $?
fi

if [ "$MODE" = "build" ]; then
    # Only build, already handled above
    exit 0
fi

echo "Usage: $0 [--pull] [--rebuild] [--exe=NAME] [build|run]"
exit 1
