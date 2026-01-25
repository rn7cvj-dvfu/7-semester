param(
    [switch]$Pull,
    [switch]$Rebuild,
    [Parameter(Position=0, ValueFromRemainingArguments=$true)]
    [string[]]$SensorArgs = @("COM10", "25", "42", "1000", "100"),
    [string[]]$LoggerArgs = @("COM11",  "./logs/all.log", "./logs/hour.log", "./logs/day.log")

)

$BuildDir = "build"
$SensorExe = "sensor.exe"
$LoggerExe = "logger.exe"
# 1. Обновление репозитория
if ($Pull) {

    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        Write-Error "Git не установлен или не в PATH"
        exit 1
    }

    git pull
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Ошибка при git pull (возможно, нет изменений)"
    }
}

# 2. Проверка инструментов
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "CMake не найден"
    exit 1
}

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    Write-Error "MinGW (gcc) не найден"
    exit 1
}

# if ($IsWindows -or $env:OS -match "Windows") {

#     $com0comInstalled = $false
    
#     try {
#         $com0comDevices = Get-WmiObject Win32_PnPEntity | Where-Object { 
#             $_.Name -match "com0com" -or $_.Service -eq "com0com"
#         }
        
#     } catch {
#     }
    
#     if (-not $com0comInstalled) {
#         Write-Warning "com0com не обнаружен!"
#         Write-Host ""
#         Write-Host "Для работы с виртуальными COM портами установите com0com:" -ForegroundColor Yellow
#         Write-Host "  1. Скачайте: https://sourceforge.net/projects/com0com/" -ForegroundColor Gray
#         Write-Host "  2. Установите драйвер от имени администратора" -ForegroundColor Gray
#         Write-Host "  3. Создайте пару портов ( COM10 ↔ COM11)" -ForegroundColor Gray
#         Write-Host ""
#         Write-Host "Или используйте реальный COM порт, если он доступен" -ForegroundColor Gray
#         Write-Host ""
#     }
# }

# 3. Сборка
if ($Rebuild) {
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }

    New-Item -ItemType Directory $BuildDir | Out-Null  

    Set-Location $BuildDir

    cmake -G "MinGW Makefiles" ..

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Ошибка генерации CMake"
        exit 1
    }


    cmake --build .
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Ошибка сборки"
        exit 1
    }

    Set-Location ..

}

Set-Location $BuildDir

# 4. Запуск
$SensorExePath = Join-Path (Get-Location) $SensorExe
$LoggerExePath = Join-Path (Get-Location) $LoggerExe

if (-not (Test-Path $SensorExePath)) {
    Write-Error "Файл sensor.exe не найден"
    Set-Location ..
    exit 1
}

Write-Host "Запуск sensor с параметрами: $SensorArgs" -ForegroundColor Yellow
Start-Process -FilePath "powershell" -ArgumentList "-NoExit", "-Command", "& '$SensorExePath' $SensorArgs"

Write-Host "Запуск logger с параметрами: $LoggerArgs" -ForegroundColor Yellow
Start-Process -FilePath "powershell" -ArgumentList "-NoExit", "-Command", "& '$LoggerExePath' $LoggerArgs"

Set-Location .. 