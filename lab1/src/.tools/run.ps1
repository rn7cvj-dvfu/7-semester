param(
    [switch]$Pull,
    [switch]$Rebuild
)

$BuildDir = "build"
$ExeName  = "lab1.exe"

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
$ExePath = Join-Path (Get-Location) $ExeName

if (-not (Test-Path $ExePath)) {
    Write-Error "Файл $ExeName не найден"
    Set-Location ..
    exit 1
}

Write-Host "Запуск $ExeName с параметрами: $Args" -ForegroundColor Yellow
& $ExePath $Args

Set-Location .. 
