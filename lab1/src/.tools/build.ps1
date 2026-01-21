$BuildDir = "build"
$ExeName  = "lab1.exe"

# 1. Обновление репозитория
Write-Host "== Обновление репозитория ==" -ForegroundColor Cyan

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "Git не установлен или не в PATH"
    exit 1
}

git pull
if ($LASTEXITCODE -ne 0) {
    Write-Error "Ошибка при git pull"
    exit 1
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

# 4. Запуск в отдельном окне
Write-Host "== Запуск программы в отдельном окне ==" -ForegroundColor Green

$ExePath = Join-Path (Get-Location) $ExeName

if (-not (Test-Path $ExePath)) {
    Write-Error "Файл $ExeName не найден"
    exit 1
}

Start-Process -FilePath $ExePath -WorkingDirectory (Get-Location)

Set-Location .. 