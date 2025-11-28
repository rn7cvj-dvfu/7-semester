# Холодный запуск Airflow с очисткой данных
param(
    [switch]$Force = $false
)

Write-Host "🔥 Холодный запуск Airflow..." -ForegroundColor Yellow

# Проверяем наличие docker-compose
if (-not (Get-Command docker-compose -ErrorAction SilentlyContinue)) {
    Write-Host "❌ docker-compose не найден. Пожалуйста, установите Docker." -ForegroundColor Red
    exit 1
}

# Остановляем контейнеры если они запущены
Write-Host "⏹️  Остановка текущих контейнеров..." -ForegroundColor Cyan
docker-compose down -v

# Удаляем старые данные PostgreSQL
Write-Host "🗑️  Удаление старых данных..." -ForegroundColor Cyan
$postgresVolume = docker volume ls --filter name=postgres_data -q
if ($postgresVolume) {
    docker volume rm $postgresVolume -f
}

# Очищаем логи
if (Test-Path "./logs") {
    Remove-Item "./logs/*" -Recurse -Force -ErrorAction SilentlyContinue
}

# Запускаем контейнеры заново
Write-Host "🚀 Запуск контейнеров..." -ForegroundColor Green
docker-compose up -d

# Ждем, пока сервисы поднимутся
Write-Host "⏳ Ожидание инициализации сервисов..." -ForegroundColor Cyan
Start-Sleep -Seconds 10


# Проверяем статус
Write-Host "✅ Холодный запуск завершен!" -ForegroundColor Green
Write-Host "🌐 Webserver доступен по адресу: http://localhost:8080" -ForegroundColor Cyan
Write-Host "📊 Credentials: admin / admin" -ForegroundColor Cyan

# Показываем статус контейнеров
Write-Host "`n📦 Статус контейнеров:" -ForegroundColor Yellow
docker-compose ps
