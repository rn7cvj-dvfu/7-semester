# Перезапуск Airflow (остановка и запуск)
Write-Host "🔄 Перезапуск Airflow..." -ForegroundColor Yellow

# Проверяем наличие docker-compose
if (-not (Get-Command docker-compose -ErrorAction SilentlyContinue)) {
    Write-Host "❌ docker-compose не найден. Пожалуйста, установите Docker." -ForegroundColor Red
    exit 1
}

# Остановляем контейнеры
Write-Host "⏹️  Остановка контейнеров..." -ForegroundColor Cyan
docker-compose down

# Запускаем контейнеры заново
Write-Host "🚀 Запуск контейнеров..." -ForegroundColor Green
docker-compose up -d

# Ждем инициализации
Write-Host "⏳ Ожидание инициализации сервисов..." -ForegroundColor Cyan
Start-Sleep -Seconds 5

Write-Host "✅ Перезапуск завершен!" -ForegroundColor Green

# Показываем статус
Write-Host "`n📦 Статус контейнеров:" -ForegroundColor Yellow
docker-compose ps
