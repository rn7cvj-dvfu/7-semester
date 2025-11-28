# Запуск Airflow
Write-Host "🚀 Запуск Airflow..." -ForegroundColor Green

# Проверяем наличие docker-compose
if (-not (Get-Command docker-compose -ErrorAction SilentlyContinue)) {
    Write-Host "❌ docker-compose не найден. Пожалуйста, установите Docker." -ForegroundColor Red
    exit 1
}

# Запускаем контейнеры
docker-compose up -d

# Ждем инициализации
Write-Host "⏳ Ожидание инициализации сервисов..." -ForegroundColor Cyan
Start-Sleep -Seconds 5

Write-Host "✅ Airflow запущен!" -ForegroundColor Green
Write-Host "🌐 Webserver доступен по адресу: http://localhost:8080" -ForegroundColor Cyan
Write-Host "📊 Credentials: admin / admin" -ForegroundColor Cyan

# Показываем статус
Write-Host "`n📦 Статус контейнеров:" -ForegroundColor Yellow
docker-compose ps
