#!/bin/bash

# Скрипт запуска Temperature Server

echo "================================"
echo "Temperature Monitoring System"
echo "================================"

# Переход в директорию build
cd "$(dirname "$0")/build"

# Проверка наличия исполняемых файлов
if [ ! -f "sensor" ] || [ ! -f "server" ]; then
    echo "Error: Executables not found!"
    echo "Please run ./build.sh first"
    exit 1
fi

# Проверка наличия socat
if ! command -v socat &> /dev/null; then
    echo "Error: socat is not installed!"
    echo "Please install socat:"
    echo "  macOS: brew install socat"
    echo "  Ubuntu/Debian: sudo apt-get install socat"
    exit 1
fi

# Параметры по умолчанию
COM_PORT_SENSOR="/tmp/vcom_sensor"
COM_PORT_SERVER="/tmp/vcom_server"
DB_PATH="./data/temperature.db"
HTTP_PORT="8080"

# Минимальная и максимальная температура для симуляции
MIN_TEMP=18
MAX_TEMP=28

# Интервал отправки данных (мс)
INTERVAL=2000
RANDOM_SHIFT=500

# Создание директории для данных
mkdir -p ./data

echo ""
echo "Configuration:"
echo "  COM Ports: $COM_PORT_SENSOR <-> $COM_PORT_SERVER"
echo "  Database: $DB_PATH"
echo "  HTTP Port: $HTTP_PORT"
echo "  Temperature Range: ${MIN_TEMP}°C - ${MAX_TEMP}°C"
echo ""

# Функция для остановки процессов при завершении
cleanup() {
    echo ""
    echo "Stopping services..."
    if [ ! -z "$SENSOR_PID" ]; then
        kill $SENSOR_PID 2>/dev/null
    fi
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
    fi
    if [ ! -z "$SOCAT_PID" ]; then
        kill $SOCAT_PID 2>/dev/null
    fi
    # Удаление файлов портов
    rm -f "$COM_PORT_SENSOR" "$COM_PORT_SERVER" 2>/dev/null
    echo "Services stopped"
    exit 0
}

trap cleanup SIGINT SIGTERM

# Создание виртуальной пары COM портов с помощью socat
echo "Creating virtual COM ports..."
socat -d -d pty,raw,echo=0,link="$COM_PORT_SENSOR" pty,raw,echo=0,link="$COM_PORT_SERVER" &
SOCAT_PID=$!

# Ждем создания портов
sleep 2

# Проверка создания портов
if [ ! -e "$COM_PORT_SENSOR" ] || [ ! -e "$COM_PORT_SERVER" ]; then
    echo "Error: Failed to create virtual COM ports!"
    cleanup
    exit 1
fi

echo "Virtual COM ports created successfully"
echo ""

# Запуск сенсора в фоновом режиме
echo "Starting temperature sensor..."
./sensor "$COM_PORT_SENSOR" $MIN_TEMP $MAX_TEMP $INTERVAL $RANDOM_SHIFT &
SENSOR_PID=$!

# Пауза для инициализации сенсора
sleep 2

# Запуск сервера
echo "Starting server..."
echo ""
./server "$COM_PORT_SERVER" "$DB_PATH" $HTTP_PORT &
SERVER_PID=$!

# Пауза для запуска сервера
sleep 3

echo ""
echo "================================"
echo "System is running!"
echo "================================"
echo ""
echo "Web Interface: http://localhost:$HTTP_PORT/"
echo ""
echo "API Endpoints:"
echo "  - Current temperature: http://localhost:$HTTP_PORT/api/current"
echo "  - Statistics: http://localhost:$HTTP_PORT/api/stats"
echo ""
echo "Press Ctrl+C to stop all services"
echo ""

# Ожидание завершения
wait
