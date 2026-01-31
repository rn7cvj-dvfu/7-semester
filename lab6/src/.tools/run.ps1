param(
    [switch]$Pull,
    [switch]$Rebuild,
    [string]$ExeName = "temperature_monitor.exe"
)


$BuildDir = "build"

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
    Push-Location $BuildDir
    cmake -G "MinGW Makefiles" ..
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Ошибка генерации CMake"
        Pop-Location
        exit 1
    }
    cmake --build .
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Ошибка сборки"
        Pop-Location
        exit 1
    }
    Pop-Location
}


Push-Location $BuildDir

$exeCandidates = @(
    "Release/$ExeName",
    $ExeName,
    "Debug/$ExeName"
)
$exePath = $null
foreach ($candidate in $exeCandidates) {
    if (Test-Path $candidate) {
        $exePath = $candidate
        break
    }
}
if (-not $exePath) {
    Write-Error "Файл $ExeName не найден в $BuildDir"
    Set-Location ..
    exit 1
}

Write-Host "Запуск $ExeName..." -ForegroundColor Green
Write-Host "Убедитесь, что сервер из lab5 запущен на http://localhost:8080"
Start-Process -FilePath "powershell" -ArgumentList "-NoExit", "-Command", ".\\$exePath"

Pop-Location
