#!/bin/bash

BUILD_DIR="build"

# Параметры по умолчанию: comPortName minValue maxValue interval randomShift
if [ $# -eq 0 ]; then
    ARGS="/dev/pts/3 0 100 1000 100"
else
    ARGS="$@"
fi

# 1. Обновление репозитория
echo "== Обновление репозитория =="

if ! command -v git &> /dev/null; then
    echo "Error: Git не установлен"
    exit 1
fi

git pull
if [ $? -ne 0 ]; then
    echo "Error: Ошибка при git pull"
    exit 1
fi

# 2. Проверка инструментов
echo "== Проверка CMake и GCC =="

if ! command -v cmake &> /dev/null; then
    echo "Error: CMake не найден"
    exit 1
fi

if ! command -v gcc &> /dev/null; then
    echo "Error: GCC не найден"
    exit 1
fi

cmake --version
gcc --version

# 3. Сборка
echo "== Сборка проекта =="

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

cmake ..
if [ $? -ne 0 ]; then
    echo "Error: Ошибка генерации CMake"
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]; then
    echo "Error: Ошибка сборки"
    exit 1
fi

# 4. Запуск
echo "== Запуск sensor =="

if [ ! -f "./sensor" ]; then
    echo "Error: Файл sensor не найден"
    cd ..
    exit 1
fi

echo "Запуск sensor с параметрами: $ARGS"
./sensor $ARGS

cd ..
