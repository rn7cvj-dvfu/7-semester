#!/bin/bash

# Просмотр логов Airflow

echo "📜 Просмотр логов Airflow..."

# Проверяем наличие docker-compose
if ! command -v docker-compose &> /dev/null; then
    echo "❌ docker-compose не найден. Пожалуйста, установите Docker."
    exit 1
fi

# Если указан аргумент, показываем логи конкретного сервиса
if [ ! -z "$1" ]; then
    echo "📋 Логи для сервиса: $1"
    docker-compose logs -f "$1"
else
    echo "📋 Логи всех сервисов"
    docker-compose logs -f
fi
