#!/bin/bash

BUILD_DIR="build"
EXE_NAME="lab3"

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

# 4. Запуск в отдельном окне
echo "== Запуск программы =="

EXE_PATH="./$EXE_NAME"

if [ ! -f "$EXE_PATH" ]; then
    echo "Error: Файл $EXE_NAME не найден"
    exit 1
fi

# Попытка запустить в новом терминале (разные варианты для разных DE)
if command -v gnome-terminal &> /dev/null; then
    gnome-terminal -- bash -c "./$EXE_NAME lab3_counter process.log increment_counter multiply_counter; echo; read -p 'Нажмите Enter для выхода...'"
elif command -v xterm &> /dev/null; then
    xterm -hold -e "./$EXE_NAME lab3_counter process.log increment_counter multiply_counter"
elif command -v konsole &> /dev/null; then
    konsole --hold -e "./$EXE_NAME lab3_counter process.log increment_counter multiply_counter"
else
    # Если нет графического терминала, запускаем в текущем
    ./$EXE_NAME lab3_counter process.log increment_counter multiply_counter
    read -p "Нажмите Enter для выхода..."
fi

cd ..
