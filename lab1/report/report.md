
# Отчет по лабораторной работе № 1

## Настройка Windows

### Установка Git

* Устновка через PowerShell
```powershell
winget install --id Git.Git -e --source winget
```

### Устанвока MinGW

* Установка MSYS2 ( https://www.msys2.org)

* Обновление MSYS2
```bash
pacman -Syu
```

* Установка MinGW 
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb mingw-w64-x86_64-make
```

* Добавление MinGW в PATH 

    C:\msys64\mingw64\bin добавить в PATH:

### Установка CMake

* Установка CMake (https://cmake.org/download/)

### Установка/Настройка VsCode

* 

## Настройка Ununtu

### Установка Oracal VirtualBox

* Установка Oracal VirtualBox (https://www.virtualbox.org/wiki/Downloads)

### Установка Ununtu

* Скачиваем образ Ubuntu (https://releases.ubuntu.com/focal/)

* Создаем машину в VirtualBox c образом Ubuntu

### Установка git

* Установка git

```
sudo apt update
sudo apt upgrade -y
sudo apt install git
```

### Установка git

* Установка git

```
sudo apt update
sudo apt upgrade -y
sudo apt install -y git
```

### Установка GCC/G++/CMake

* Установка GCC/G++/CMake
```
sudo apt update
sudo apt upgrade -y
sudo apt install -y build-essential
sudo apt install -y cmake
```

## Настройка проекта

* Создаем [main.cpp](/lab1/src/main.cpp)
```
#include <iostream>

int main() {
    std::cout << "Hello, world" << std::endl;
    std::cin.get();
    return 0;
}

```


* Создаем [CMakeLists.txt](/lab1/src/CMakeLists.txt)
```
cmake_minimum_required(VERSION 3.10)
project(lab1)

set(CMAKE_CXX_STANDARD 17)

add_executable(lab1 main.cpp)
```

* Создаем скрипт для автоматизирующие обновление исходных кодов проекта с GIT-репозитория, сборку и компиляцию проекта для Windows [build.ps1](/lab1/src/.tools/build.ps1)

```
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
```

* * Создаем скрипт для автоматизирующие обновление исходных кодов проекта с GIT-репозитория, сборку и компиляцию проекта для Ununtu [build.ps1](/lab1/src/.tools/build.sh)
