#!/bin/bash

# Создание виртуальной пары COM портов для Linux

echo "=== Настройка виртуальных COM портов ==="

# Проверяем наличие socat
if ! command -v socat &> /dev/null; then
    echo "Error: socat не установлен"
    echo "Установите: sudo apt-get install socat"
    exit 1
fi

# Создаем виртуальную пару портов
# /tmp/vcom_sensor и /tmp/vcom_logger будут подключены друг к другу
echo "Создание виртуальной пары COM портов..."

# Запускаем socat в фоне
socat -d -d PTY,link=/tmp/vcom_sensor PTY,link=/tmp/vcom_logger &
SOCAT_PID=$!

# Даем время на создание портов
sleep 1

if [ -e /tmp/vcom_sensor ] && [ -e /tmp/vcom_logger ]; then
    echo "✓ Виртуальные COM порты созданы успешно:"
    echo "  - Датчик (писатель):  /tmp/vcom_sensor"
    echo "  - Логгер (читатель):  /tmp/vcom_logger"
    echo ""
    echo "Параметры для run.sh:"
    echo "  --sensor-args /tmp/vcom_sensor 0 100 1000 100"
    echo "  --logger-args /tmp/vcom_logger ./logs/all.log ./logs/hour.log ./logs/day.log"
    echo ""
    echo "PID процесса socat: $SOCAT_PID"
    echo "Для остановки: kill $SOCAT_PID"
else
    echo "Error: Не удалось создать виртуальные портты"
    kill $SOCAT_PID
    exit 1
fi

# Оставляем процесс запущенным
wait $SOCAT_PID
