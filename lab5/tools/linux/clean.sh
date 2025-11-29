#!/bin/bash

# Полная очистка Airflow

echo "🗑️  Полная очистка Airflow..."

# Проверяем наличие docker-compose
if ! command -v docker-compose &> /dev/null; then
    echo "❌ docker-compose не найден. Пожалуйста, установите Docker."
    exit 1
fi

# Остановляем и удаляем контейнеры вместе с томами
echo "⏹️  Остановка и удаление контейнеров..."
docker-compose down --volumes --rmi all

# Удаляем все volumes Airflow
echo "🗑️  Удаление всех томов..."
docker volume prune -f

# Очищаем директории логов и данных
if [ -d "./logs" ]; then
    echo "📁 Очистка логов..."
    rm -rf ./logs/*
fi

if [ -d "./data" ]; then
    echo "📁 Очистка данных..."
    rm -rf ./data/*
fi

echo "✅ Полная очистка завершена!"
echo "Для повторного запуска используйте: ./cold-start.sh"
