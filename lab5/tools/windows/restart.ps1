Write-Host "Restarting Airflow..." -ForegroundColor Yellow
docker compose down
Write-Host "Waiting 3 seconds..." -ForegroundColor Gray
Start-Sleep -Seconds 3
docker compose up -d postgres
Write-Host "Waiting for postgres to be ready..." -ForegroundColor Yellow
Start-Sleep -Seconds 5
docker compose up -d webserver scheduler
Write-Host "Airflow restarted!" -ForegroundColor Green
Write-Host "Access webserver at http://localhost:8080" -ForegroundColor Cyan
