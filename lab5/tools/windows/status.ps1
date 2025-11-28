Write-Host "Checking Airflow status..." -ForegroundColor Cyan
docker compose ps
Write-Host "`nWebserver URL: http://localhost:8080" -ForegroundColor Green
