#!/bin/bash

# Параметры
PULL=false
REBUILD=false
ARGS=()

# Парсинг аргументов
while [[ $# -gt 0 ]]; do
    case $1 in
        --pull)
            PULL=true
            shift
            ;;
        --rebuild)
            REBUILD=true
            shift
            ;;
        *)
            ARGS+=("$1")
            shift
            ;;
    esac
done

BUILD_DIR="build"
EXE_NAME="lab2"

# 1. Обновление репозитория
if [ "$PULL" = true ]; then

    if ! command -v git &> /dev/null; then
        echo "Error: Git не установлен"
        exit 1
    fi

    git pull
    if [ $? -ne 0 ]; then
        echo "Warning: Ошибка при git pull (возможно, нет изменений)"
    fi
fi

# 2. Проверка инструментов
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake не найден"
    exit 1
fi

if ! command -v gcc &> /dev/null; then
    echo "Error: GCC не найден"
    exit 1
fi

# 3. Сборка
if [ "$REBUILD" = true ]; then
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi

    mkdir -p "$BUILD_DIR"

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

    cd ..

fi

cd "$BUILD_DIR"

# 4. Запуск
EXE_PATH="./$EXE_NAME"

if [ ! -f "$EXE_PATH" ]; then
    echo "Error: Файл $EXE_NAME не найден"
    cd ..
    exit 1
fi

echo "Запуск $EXE_NAME с параметрами: ${ARGS[@]}"
"$EXE_PATH" "${ARGS[@]}"

cd ..
