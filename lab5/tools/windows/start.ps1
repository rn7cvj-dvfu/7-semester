Write-Host "Starting Airflow..." -ForegroundColor Green
docker compose up -d postgres
Write-Host "Waiting for postgres to be ready..." -ForegroundColor Yellow
Start-Sleep -Seconds 5
docker compose up -d webserver scheduler
Write-Host "Airflow started!" -ForegroundColor Green
Write-Host "Access webserver at http://localhost:8080" -ForegroundColor Cyan
