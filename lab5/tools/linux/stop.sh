#!/bin/bash

# Остановка Airflow

echo "⏹️  Остановка Airflow..."

# Проверяем наличие docker-compose
if ! command -v docker-compose &> /dev/null; then
    echo "❌ docker-compose не найден. Пожалуйста, установите Docker."
    exit 1
fi

# Остановляем контейнеры
docker-compose down

echo "✅ Airflow остановлен!"

# Показываем статус
echo ""
echo "📦 Статус контейнеров:"
docker-compose ps
