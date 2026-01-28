param(
    [switch]$Pull,
    [switch]$Rebuild,
    [string[]]$ServerArgs = @("COM10", "./data/temperature.db", "8080"),
    [string[]]$SensorArgs = @("COM11", "20", "40", "1000", "100")
)

# Справка по параметрам:
# -Rebuild         Пересборить проект
# -Pull            Обновить репозиторий перед сборкой
# -ServerArgs      Аргументы для сервера (default: COM10, ./data/temperature.db, 8080)
#                  Параметры: <comPort> <databasePath> <httpPort>
# -SensorArgs      Аргументы для сенсора (default: COM11, 20, 40, 1000, 100)
#                  Параметры: <comPort> <minValue> <maxValue> <interval> <randomShift>
#
# Пример использования:
# .\run.ps1 -Rebuild -ServerArgs COM10, "./data/temp.db", 8888
# .\run.ps1 -SensorArgs COM11, 15, 35, 2000, 50

$BuildDir = "build"
$SensorExe = "sensor.exe"
$ServerExe = "server.exe"

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

Write-Host "CMake found: $(cmake --version | Select-Object -First 1)"

# 3. Сборка
if ($Rebuild) {
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
    }

    New-Item -ItemType Directory $BuildDir | Out-Null  

    Set-Location $BuildDir

    cmake -G "Visual Studio 18 2026" ..

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Ошибка генерации CMake"
        exit 1
    }

    cmake --build . --config Release
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Ошибка сборки"
        exit 1
    }

    Set-Location ..
}

Set-Location $BuildDir

# 4. Запуск
$SensorExePath = Join-Path (Get-Location) "Release\$SensorExe"
$ServerExePath = Join-Path (Get-Location) "Release\$ServerExe"

if (-not (Test-Path $SensorExePath)) {
    Write-Error "Файл $SensorExe не найден в $SensorExePath"
    Set-Location ..
    exit 1
}

if (-not (Test-Path $ServerExePath)) {
    Write-Error "Файл $ServerExe не найден в $ServerExePath"
    Set-Location ..
    exit 1
}

Write-Host "Запуск server с параметрами: $ServerArgs" -ForegroundColor Green
Start-Process -FilePath "powershell" -ArgumentList "-NoExit", "-Command", "& '$ServerExePath' $ServerArgs"

Write-Host "Запуск sensor с параметрами: $SensorArgs" -ForegroundColor Green
Start-Process -FilePath "powershell" -ArgumentList "-NoExit", "-Command", "& '$SensorExePath' $SensorArgs"

Set-Location ..
