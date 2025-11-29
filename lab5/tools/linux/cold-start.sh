#!/bin/bash

# Холодный запуск Airflow с очисткой данных

echo "🔥 Холодный запуск Airflow..."

# Проверяем наличие docker-compose
if ! command -v docker-compose &> /dev/null; then
    echo "❌ docker-compose не найден. Пожалуйста, установите Docker."
    exit 1
fi

# Остановляем контейнеры если они запущены
echo "⏹️  Остановка текущих контейнеров..."
docker-compose down -v

# Удаляем старые данные PostgreSQL
echo "🗑️  Удаление старых данных..."
postgres_volume=$(docker volume ls --filter name=postgres_data -q)
if [ ! -z "$postgres_volume" ]; then
    docker volume rm "$postgres_volume" -f
fi

# Очищаем логи
if [ -d "./logs" ]; then
    rm -rf ./logs/*
fi

# Запускаем контейнеры заново
echo "🚀 Запуск контейнеров..."
docker-compose up -d

# Ждем, пока сервисы поднимутся
echo "⏳ Ожидание инициализации сервисов..."
sleep 10

# Проверяем статус
echo "✅ Холодный запуск завершен!"
echo "🌐 Webserver доступен по адресу: http://localhost:8080"
echo "📊 Credentials: admin / admin"

# Показываем статус контейнеров
echo ""
echo "📦 Статус контейнеров:"
docker-compose ps
