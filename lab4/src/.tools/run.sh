#!/bin/bash

BUILD_DIR="build"
SENSOR_EXE="sensor"
LOGGER_EXE="logger"


PULL=false
REBUILD=false
SENSOR_ARGS=()
LOGGER_ARGS=()


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
        --sensor-args)
            shift
            while [[ $# -gt 0 ]] && [[ $1 != --* ]]; do
                SENSOR_ARGS+=("$1")
                shift
            done
            ;;
        --logger-args)
            shift
            while [[ $# -gt 0 ]] && [[ $1 != --* ]]; do
                LOGGER_ARGS+=("$1")
                shift
            done
            ;;
        *)
            shift
            ;;
    esac
done

if [ ${#SENSOR_ARGS[@]} -eq 0 ]; then
    SENSOR_ARGS=("/tmp/vcom_sensor" "0" "100" "1000" "100")
fi

if [ ${#LOGGER_ARGS[@]} -eq 0 ]; then
    LOGGER_ARGS=("/tmp/vcom_logger" "./logs/all.log" "./logs/hour.log" "./logs/day.log")
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

echo "== Проверка CMake и GCC =="

if ! command -v cmake &> /dev/null; then
    echo "Error: CMake не найден"
    exit 1
fi

if ! command -v gcc &> /dev/null; then
    echo "Error: GCC не найден"
    exit 1
fi

echo "CMake $(cmake --version | head -n1)"
echo "GCC $(gcc --version | head -n1)"


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

    cmake --build .
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
        cmake --build .
        cd ..
    fi
fi

cd "$BUILD_DIR"


echo "== Запуск приложений =="


if [ ! -e "${SENSOR_ARGS[0]}" ] || [ ! -e "${LOGGER_ARGS[0]}" ]; then
    echo "Warning: Виртуальные COM порты не найдены!"
    echo "Создайте их в отдельном терминале:"
    echo "  ./.tools/setup_vcom.sh"
    echo ""
   exit 1
fi

if [ ! -f "./$SENSOR_EXE" ]; then
    echo "Error: Файл $SENSOR_EXE не найден"
    cd ..
    exit 1
fi

if [ ! -f "./$LOGGER_EXE" ]; then
    echo "Error: Файл $LOGGER_EXE не найден"
    cd ..
    exit 1
fi

echo "Запуск $SENSOR_EXE с параметрами: ${SENSOR_ARGS[@]}"
./$SENSOR_EXE "${SENSOR_ARGS[@]}" &
SENSOR_PID=$!

echo "Запуск $LOGGER_EXE с параметрами: ${LOGGER_ARGS[@]}"
./$LOGGER_EXE "${LOGGER_ARGS[@]}" &
LOGGER_PID=$!

wait $SENSOR_PID $LOGGER_PID

cd ..
