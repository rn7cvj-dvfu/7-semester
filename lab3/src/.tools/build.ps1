$BuildDir = "build"
$ExeName  = "lab3.exe"

# 1. Обновление репозитория
Write-Host "== Обновление репозитория ==" -ForegroundColor Cyan

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "Git не установлен или не в PATH"
    exit 1
}

git pull
if ($LASTEXITCODE -ne 0) {
    Write-Warning "Ошибка при git pull (возможно, нет изменений)"
}

# 2. Проверка инструментов
Write-Host "== Проверка CMake и MinGW ==" -ForegroundColor Cyan

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "CMake не найден"
    exit 1
}

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    Write-Error "MinGW (gcc) не найден"
    exit 1
}

cmake --version
gcc --version

# 3. Сборка
Write-Host "== Сборка проекта ==" -ForegroundColor Cyan

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory $BuildDir | Out-Null
}

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

# 4. Запуск
Write-Host "== Запуск программы ==" -ForegroundColor Green

$ExePath = Join-Path (Get-Location) $ExeName

if (-not (Test-Path $ExePath)) {
    Write-Error "Файл $ExeName не найден"
    exit 1
}

Write-Host "Запуск $ExeName..." -ForegroundColor Yellow
Write-Host "Параметры: lab3_counter process.log increment_counter multiply_counter" -ForegroundColor Yellow
& $ExePath "lab3_counter" "./logs/process.log" "./increment_counter.exe" "./multiply_counter.exe"

Set-Location .. 