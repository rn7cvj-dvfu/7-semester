#!/bin/bash

# Скрипт сборки проекта lab5

echo "================================"
echo "Building Temperature Server"
echo "================================"

# Переход в директорию с исходниками
cd "$(dirname "$0")"

# Создание директории для сборки
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Проверка наличия SQLite3
if ! command -v pkg-config &> /dev/null || ! pkg-config --exists sqlite3; then
    echo "Warning: SQLite3 not found via pkg-config"
    echo "Trying to build anyway..."
fi

# Запуск CMake
echo ""
echo "Running CMake..."
cmake ..

if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed!"
    exit 1
fi

# Сборка проекта
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
echo "Executables:"
echo "  - sensor: ./build/sensor"
echo "  - server: ./build/server"
echo ""
echo "To run the server:"
echo "  ./run.sh"
echo ""
