# Очистка контейнеров и данных Airflow
param(
    [switch]$Force = $false
)

Write-Host "🧹 Очистка Airflow..." -ForegroundColor Yellow

# Проверяем наличие docker-compose
if (-not (Get-Command docker-compose -ErrorAction SilentlyContinue)) {
    Write-Host "❌ docker-compose не найден. Пожалуйста, установите Docker." -ForegroundColor Red
    exit 1
}

if (-not $Force) {
    $response = Read-Host "⚠️  Это удалит ВСЕ контейнеры, тома и логи. Продолжить? (y/n)"
    if ($response -ne 'y') {
        Write-Host "❌ Очистка отменена." -ForegroundColor Red
        exit 0
    }
}

Write-Host "⏹️  Остановка контейнеров..." -ForegroundColor Cyan
docker-compose down -v

Write-Host "🗑️  Удаление томов..." -ForegroundColor Cyan
$volumes = docker volume ls --filter name=airflow -q
if ($volumes) {
    $volumes | ForEach-Object {
        docker volume rm $_ -f
    }
}

Write-Host "🗑️  Удаление логов..." -ForegroundColor Cyan
if (Test-Path "./logs") {
    Remove-Item "./logs/*" -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host "✅ Очистка завершена!" -ForegroundColor Green
Write-Host "Используйте './tools/windows/cold-start.ps1' для нового запуска" -ForegroundColor Cyan
