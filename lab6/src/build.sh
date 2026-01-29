#!/bin/bash

echo "================================"
echo "Building Temperature Monitor GUI"
echo "================================"

cd "$(dirname "$0")"

# Проверка наличия Qt
if ! command -v qmake &> /dev/null && ! command -v qmake6 &> /dev/null; then
    echo "Error: Qt not found!"
    echo "Please install Qt:"
    echo "  macOS: brew install qt"
    echo "  Ubuntu/Debian: sudo apt-get install qt6-base-dev qt6-charts-dev"
    exit 1
fi

# Создание директории для сборки
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

echo ""
echo "Running CMake..."
cmake ..

if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed!"
    exit 1
fi

echo ""
echo "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

if [ $? -ne 0 ]; then
    echo "Error: Build failed!"
    exit 1
fi

echo ""
echo "================================"
echo "Build completed successfully!"
echo "================================"
echo ""
echo "Executable: ./build/temperature_monitor"
echo ""
echo "To run:"
echo "  ./run.sh"
echo ""
