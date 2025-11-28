# Остановка Airflow
Write-Host "⏹️  Остановка Airflow..." -ForegroundColor Yellow

# Проверяем наличие docker-compose
if (-not (Get-Command docker-compose -ErrorAction SilentlyContinue)) {
    Write-Host "❌ docker-compose не найден. Пожалуйста, установите Docker." -ForegroundColor Red
    exit 1
}

# Остановляем контейнеры
docker-compose down

Write-Host "✅ Airflow остановлен!" -ForegroundColor Green

# Показываем статус
Write-Host "`n📦 Статус контейнеров:" -ForegroundColor Yellow
docker-compose ps
