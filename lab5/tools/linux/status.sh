#!/bin/bash

# Проверка статуса Airflow

echo "📊 Статус Airflow:"

# Проверяем наличие docker-compose
if ! command -v docker-compose &> /dev/null; then
    echo "❌ docker-compose не найден. Пожалуйста, установите Docker."
    exit 1
fi

# Показываем статус контейнеров
docker-compose ps

echo ""
echo "🔍 Проверка доступности сервисов:"

# Проверяем webserver
if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/health | grep -q "200"; then
    echo "✅ Webserver: доступен (http://localhost:8080)"
else
    echo "❌ Webserver: недоступен"
fi

# Проверяем PostgreSQL
if docker-compose ps postgres | grep -q "Up"; then
    echo "✅ PostgreSQL: работает"
else
    echo "❌ PostgreSQL: недоступна"
fi

# Проверяем Redis
if docker-compose ps redis | grep -q "Up"; then
    echo "✅ Redis: работает"
else
    echo "❌ Redis: недоступен"
fi

# Показываем логи
echo ""
echo "📜 Для просмотра логов используйте:"
echo "docker-compose logs -f [service_name]"
