# Справка по скриптам управления Airflow

Write-Host "
╔════════════════════════════════════════════════════════════════╗
║          📚 Справка по скриптам управления Airflow             ║
╚════════════════════════════════════════════════════════════════╝
" -ForegroundColor Cyan

Write-Host "🚀 ЗАПУСК И ОСТАНОВКА:" -ForegroundColor Yellow
Write-Host "  ./cold-start.ps1   - Холодный запуск (очистка всех данных)" -ForegroundColor White
Write-Host "  ./start.ps1        - Обычный запуск" -ForegroundColor White
Write-Host "  ./restart.ps1      - Перезапуск контейнеров" -ForegroundColor White
Write-Host "  ./stop.ps1         - Остановка Airflow" -ForegroundColor White

Write-Host "`n📊 МОНИТОРИНГ И ЛОГИ:" -ForegroundColor Yellow
Write-Host "  ./status.ps1       - Проверка статуса сервисов" -ForegroundColor White
Write-Host "  ./logs.ps1         - Просмотр логов" -ForegroundColor White
Write-Host "    -Service webserver   - Логи webserver" -ForegroundColor Gray
Write-Host "    -Service scheduler   - Логи scheduler" -ForegroundColor Gray
Write-Host "    -Service worker      - Логи worker" -ForegroundColor Gray
Write-Host "    -Follow              - Следить за логами в реальном времени" -ForegroundColor Gray

Write-Host "`n🧹 ОЧИСТКА:" -ForegroundColor Yellow
Write-Host "  ./clean.ps1        - Удаление всех контейнеров и данных" -ForegroundColor White
Write-Host "    -Force           - Пропустить подтверждение" -ForegroundColor Gray

Write-Host "`n📝 ПРИМЕРЫ:" -ForegroundColor Yellow
Write-Host "  # Холодный запуск Airflow" -ForegroundColor Green
Write-Host "  ./cold-start.ps1" -ForegroundColor Gray
Write-Host ""
Write-Host "  # Просмотр логов webserver в реальном времени" -ForegroundColor Green
Write-Host "  ./logs.ps1 -Service webserver -Follow" -ForegroundColor Gray
Write-Host ""
Write-Host "  # Перезагрузка всех компонентов" -ForegroundColor Green
Write-Host "  ./restart.ps1" -ForegroundColor Gray
Write-Host ""
Write-Host "  # Проверка статуса" -ForegroundColor Green
Write-Host "  ./status.ps1" -ForegroundColor Gray

Write-Host "`n🌐 ДОСТУПНЫЕ СЕРВИСЫ:" -ForegroundColor Yellow
Write-Host "  Webserver: http://localhost:8080" -ForegroundColor Cyan
Write-Host "  PostgreSQL: localhost:5432" -ForegroundColor Cyan
Write-Host "  Redis: localhost:6379" -ForegroundColor Cyan
Write-Host "  Credentials (webserver): admin / admin" -ForegroundColor Cyan

Write-Host "`n" -ForegroundColor Yellow
