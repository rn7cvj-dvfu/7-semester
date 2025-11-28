# Проверка статуса Airflow
Write-Host "📊 Статус Airflow:" -ForegroundColor Cyan

# Проверяем наличие docker-compose
if (-not (Get-Command docker-compose -ErrorAction SilentlyContinue)) {
    Write-Host "❌ docker-compose не найден. Пожалуйста, установите Docker." -ForegroundColor Red
    exit 1
}

# Показываем статус контейнеров
docker-compose ps

Write-Host "`n🔍 Проверка доступности сервисов:" -ForegroundColor Yellow

# Проверяем webserver
$webserverResponse = $null
try {
    $webserverResponse = Invoke-WebRequest -Uri "http://localhost:8080/health" -ErrorAction SilentlyContinue
    if ($webserverResponse.StatusCode -eq 200) {
        Write-Host "✅ Webserver: доступен (http://localhost:8080)" -ForegroundColor Green
    }
} catch {
    Write-Host "❌ Webserver: недоступен" -ForegroundColor Red
}

# Проверяем PostgreSQL
try {
    $pgContainer = docker-compose ps -q postgres 2>$null
    if ($pgContainer) {
        Write-Host "✅ PostgreSQL: работает" -ForegroundColor Green
    }
} catch {
    Write-Host "❌ PostgreSQL: недоступна" -ForegroundColor Red
}

# Проверяем Redis
try {
    $redisContainer = docker-compose ps -q redis 2>$null
    if ($redisContainer) {
        Write-Host "✅ Redis: работает" -ForegroundColor Green
    }
} catch {
    Write-Host "❌ Redis: недоступен" -ForegroundColor Red
}

# Показываем логи
Write-Host "`n📜 Последние логи:" -ForegroundColor Yellow
Write-Host "Используйте для просмотра логов: docker-compose logs -f [service_name]" -ForegroundColor Cyan
