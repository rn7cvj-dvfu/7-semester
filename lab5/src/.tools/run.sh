#!/bin/bash

BUILD_DIR="build"
SENSOR_EXE="sensor"
SERVER_EXE="server"

SERVER_ARGS=("/tmp/vcom_logger" "./data/temperature.db" "8080")
SENSOR_ARGS=()

PULL=false
REBUILD=false

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
        --server-args)
            shift
            SERVER_ARGS=()
            while [[ $# -gt 0 ]] && [[ $1 != --* ]]; do
                SERVER_ARGS+=("$1")
                shift
            done
            ;;
        --sensor-args)
            shift
            while [[ $# -gt 0 ]] && [[ $1 != --* ]]; do
                SENSOR_ARGS+=("$1")
                shift
            done
            ;;
        *)
            shift
            ;;
    esac
done

if [ ${#SENSOR_ARGS[@]} -eq 0 ]; then
    SENSOR_ARGS=("/tmp/vcom_sensor" "20" "40" "1000" "100")
fi

if [ "$PULL" = true ]; then
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
fi

echo "== Проверка CMake =="

if ! command -v cmake &> /dev/null; then
    echo "Error: CMake не найден"
    exit 1
fi

echo "CMake $(cmake --version | head -n1)"

if [ "$REBUILD" = true ]; then
    echo "== Сборка проекта =="

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

    cmake --build . --config Release
    if [ $? -ne 0 ]; then
        echo "Error: Ошибка сборки"
        exit 1
    fi

    cd ..
else
    if [ ! -d "$BUILD_DIR" ]; then
        echo "== Сборка проекта (автоматическая) =="
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake ..
        cmake --build . --config Release
        cd ..
    fi
fi

cd "$BUILD_DIR"

echo "== Запуск приложений =="

if [ ! -f "./$SERVER_EXE" ]; then
    echo "Error: Файл $SERVER_EXE не найден"
    cd ..
    exit 1
fi

if [ ! -f "./$SENSOR_EXE" ]; then
    echo "Error: Файл $SENSOR_EXE не найден"
    cd ..
    exit 1
fi

sleep 2

./$SERVER_EXE "${SERVER_ARGS[@]}"

./$SENSOR_EXE "${SENSOR_ARGS[@]}"

cd ..
