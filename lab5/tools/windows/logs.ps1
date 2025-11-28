# Просмотр логов Airflow
param(
    [Parameter(Mandatory=$false)]
    [ValidateSet('webserver', 'scheduler', 'worker', 'postgres', 'redis', 'init-airflow')]
    [string]$Service = '',
    [switch]$Follow = $false
)

Write-Host "📜 Просмотр логов Airflow..." -ForegroundColor Yellow

# Проверяем наличие docker-compose
if (-not (Get-Command docker-compose -ErrorAction SilentlyContinue)) {
    Write-Host "❌ docker-compose не найден. Пожалуйста, установите Docker." -ForegroundColor Red
    exit 1
}

# Если сервис не указан, показываем все логи
if ([string]::IsNullOrEmpty($Service)) {
    Write-Host "Просмотр логов всех сервисов..." -ForegroundColor Cyan
    if ($Follow) {
        docker-compose logs -f
    } else {
        docker-compose logs --tail=50
    }
} else {
    Write-Host "Просмотр логов сервиса: $Service" -ForegroundColor Cyan
    if ($Follow) {
        docker-compose logs -f $Service
    } else {
        docker-compose logs --tail=50 $Service
    }
}
