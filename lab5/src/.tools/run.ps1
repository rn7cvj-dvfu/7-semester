param(
    [switch]$Pull,
    [switch]$Rebuild,
    [string[]]$ServerArgs = @("COM10", "./data/temperature.db", "8080"),
    [string[]]$SensorArgs = @("COM11", "20", "40", "1000", "100")
)


$BuildDir = "build"
$SensorExe = "sensor.exe"
$ServerExe = "server.exe"

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

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "CMake не найден"
    exit 1
}

Write-Host "CMake found: $(cmake --version | Select-Object -First 1)"

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

$SensorExePath = Join-Path (Get-Location) $SensorExe
$ServerExePath = Join-Path (Get-Location) $ServerExe

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
