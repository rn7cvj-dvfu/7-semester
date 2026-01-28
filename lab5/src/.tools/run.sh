#!/bin/bash

# Справка по параметрам:
# --rebuild              Пересборить проект
# --pull                 Обновить репозиторий перед сборкой
# --server-args <args>   Аргументы для сервера (default: /dev/pts/3, ./data/temperature.db, 8080)
#                        Параметры: <comPort> <databasePath> <httpPort>
# --sensor-args <args>   Аргументы для сенсора (default: /dev/pts/4, 20, 40, 1000, 100)
#                        Параметры: <comPort> <minValue> <maxValue> <interval> <randomShift>
#
# Пример использования:
# ./run.sh --rebuild --server-args /dev/pts/3 ./data/temp.db 8888
# ./run.sh --sensor-args /dev/pts/4 15 35 2000 50

BUILD_DIR="build"
SENSOR_EXE="sensor"
SERVER_EXE="server"

# Параметры по умолчанию
SERVER_ARGS=("/dev/pts/3" "./data/temperature.db" "8080")
SENSOR_ARGS=()

# Параметры
PULL=false
REBUILD=false

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

# Значения по умолчанию если не указаны
if [ ${#SENSOR_ARGS[@]} -eq 0 ]; then
    SENSOR_ARGS=("/dev/pts/4" "20" "40" "1000" "100")
fi

# 1. Обновление репозитория
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

# 2. Проверка инструментов
echo "== Проверка CMake =="

if ! command -v cmake &> /dev/null; then
    echo "Error: CMake не найден"
    exit 1
fi

echo "CMake $(cmake --version | head -n1)"

# 3. Сборка
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
    # Проверяем, есть ли уже build директория
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

# 4. Запуск
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

echo "Запуск $SERVER_EXE с параметрами: ${SERVER_ARGS[@]}"
./$SERVER_EXE "${SERVER_ARGS[@]}" &
SERVER_PID=$!

sleep 2

echo "Запуск $SENSOR_EXE с параметрами: ${SENSOR_ARGS[@]}"
./$SENSOR_EXE "${SENSOR_ARGS[@]}" &
SENSOR_PID=$!

echo ""
echo "Server PID: $SERVER_PID"
echo "Sensor PID: $SENSOR_PID"
echo ""
echo "Нажмите Ctrl+C для остановки приложений"
echo ""

# Ожидание завершения процессов
wait $SERVER_PID $SENSOR_PID

cd ..
