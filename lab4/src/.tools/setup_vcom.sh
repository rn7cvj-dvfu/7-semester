#!/bin/bash

echo "=== Настройка виртуальных COM портов ==="

# Проверяем наличие socat
if ! command -v socat &> /dev/null; then
    echo "Error: socat не установлен"
    echo "Установите: sudo apt-get install socat"
    exit 1
fi


echo "Создание виртуальной пары COM портов..."

socat -d -d PTY,link=/tmp/vcom_sensor PTY,link=/tmp/vcom_logger &
SOCAT_PID=$!

sleep 1

if [ -e /tmp/vcom_sensor ] && [ -e /tmp/vcom_logger ]; then
    echo "✓ Виртуальные COM порты созданы успешно:"
    echo "  - Датчик:  /tmp/vcom_sensor"
    echo "  - Логгер:  /tmp/vcom_logger"
    echo "PID процесса socat: $SOCAT_PID"
    echo "Для остановки: kill $SOCAT_PID"
else
    echo "Error: Не удалось создать виртуальные портты"
    kill $SOCAT_PID
    exit 1
fi

# Оставляем процесс запущенным
wait $SOCAT_PID
