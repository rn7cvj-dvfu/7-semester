#!/bin/bash

# Перезапуск Airflow (остановка и запуск)

echo "🔄 Перезапуск Airflow..."

# Проверяем наличие docker-compose
if ! command -v docker-compose &> /dev/null; then
    echo "❌ docker-compose не найден. Пожалуйста, установите Docker."
    exit 1
fi

# Остановляем контейнеры
echo "⏹️  Остановка контейнеров..."
docker-compose down

# Запускаем контейнеры заново
echo "🚀 Запуск контейнеров..."
docker-compose up -d

# Ждем инициализации
echo "⏳ Ожидание инициализации сервисов..."
sleep 5

echo "✅ Перезапуск завершен!"

# Показываем статус
echo ""
echo "📦 Статус контейнеров:"
docker-compose ps
