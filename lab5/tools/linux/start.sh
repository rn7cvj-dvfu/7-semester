#!/bin/bash

# Запуск Airflow

echo "🚀 Запуск Airflow..."

# Проверяем наличие docker-compose
if ! command -v docker-compose &> /dev/null; then
    echo "❌ docker-compose не найден. Пожалуйста, установите Docker."
    exit 1
fi

# Запускаем контейнеры
docker-compose up -d

# Ждем инициализации
echo "⏳ Ожидание инициализации сервисов..."
sleep 5

echo "✅ Airflow запущен!"
echo "🌐 Webserver доступен по адресу: http://localhost:8080"
echo "📊 Credentials: admin / admin"

# Показываем статус
echo ""
echo "📦 Статус контейнеров:"
docker-compose ps
