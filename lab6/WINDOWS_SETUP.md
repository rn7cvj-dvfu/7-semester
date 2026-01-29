# Windows Setup Guide - Temperature Monitor GUI

Подробная инструкция по сборке и запуску GUI приложения на Windows.

## Требования

### 1. CMake
- **Версия:** >= 3.16
- **Скачать:** https://cmake.org/download/
- При установке выберите "Add CMake to system PATH"

### 2. Компилятор

Выберите один из вариантов:

#### Вариант A: MinGW (Рекомендуется для простоты)
- **Скачать:** https://winlibs.com/ (MinGW-w64 with GCC)
- Распаковать и добавить `bin` в PATH
- Проверка: `gcc --version`

#### Вариант B: Visual Studio
- **Visual Studio 2019 или 2022** (Community Edition бесплатна)
- **Скачать:** https://visualstudio.microsoft.com/downloads/
- При установке выберите "Desktop development with C++"

### 3. Qt Framework

#### Установка Qt

1. **Скачать Qt Online Installer:**
   - https://www.qt.io/download-qt-installer

2. **Запустить установку:**
   - Создать бесплатный Qt Account
   - Выбрать компоненты для установки:
     - Qt 6.5.x или 6.6.x
       - MSVC 2019 64-bit (для Visual Studio)
       - MinGW 11.2.0 64-bit (для MinGW)
     - Qt Charts
     - Developer and Designer Tools (опционально)

3. **Типичный путь установки:**
   ```
   C:\Qt\6.5.0\msvc2019_64\
   или
   C:\Qt\6.5.0\mingw_64\
   ```

4. **Настроить переменные окружения:**

   Добавьте в PATH:
   ```
   C:\Qt\6.5.0\msvc2019_64\bin
   ```
   
   И установите (опционально):
   ```
   set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
   set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
   ```

## Быстрый старт

### 1. Запустить сервер (lab5)

В одном терминале:
```cmd
cd lab5\src
build.bat
run.bat
```

**Важно:** Сервер должен работать на http://localhost:8080

### 2. Собрать GUI приложение

В другом терминале:
```cmd
cd lab6\src
build.bat
```

Скрипт автоматически:
- Проверит наличие CMake и Qt
- Создаст директорию build
- Запустит CMake с правильным генератором
- Соберет проект

### 3. Запустить приложение

```cmd
run.bat
```

## Ручная сборка

### С MinGW

```cmd
cd lab6\src
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\6.5.0\mingw_64
cmake --build . --config Release
```

Исполняемый файл: `build\temperature_monitor.exe`

### С Visual Studio

```cmd
cd lab6\src
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
cmake --build . --config Release
```

Исполняемый файл: `build\Release\temperature_monitor.exe`

### Открыть в Visual Studio

1. Открыть CMakeLists.txt в Visual Studio
2. Visual Studio автоматически настроит проект
3. Build > Build Solution (Ctrl+Shift+B)
4. Debug > Start Without Debugging (Ctrl+F5)

## Решение проблем

### 1. CMake не найден

**Симптом:**
```
'cmake' is not recognized as an internal or external command
```

**Решение:**
- Переустановите CMake с опцией "Add to PATH"
- Или добавьте вручную: `C:\Program Files\CMake\bin`
- Перезапустите командную строку

### 2. Qt не найден

**Симптом:**
```
Could not find a package configuration file provided by "Qt6"
```

**Решение:**

Установите переменную окружения:
```cmd
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
```

Или укажите напрямую в CMake:
```cmd
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
```

### 3. Qt Charts не найден

**Симптом:**
```
Could not find a package configuration file provided by "Qt6Charts"
```

**Решение:**
- Запустите Qt Maintenance Tool
- Добавьте компонент "Qt Charts"
- Переустановите для нужной версии Qt

### 4. Приложение не запускается - отсутствуют DLL

**Симптом:**
```
The code execution cannot proceed because Qt6Core.dll was not found
```

**Решение:**

Добавьте Qt bin в PATH:
```cmd
set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%PATH%
```

Или скопируйте необходимые DLL в папку с exe:
- Qt6Core.dll
- Qt6Gui.dll
- Qt6Widgets.dll
- Qt6Network.dll
- Qt6Charts.dll

Используйте windeployqt для автоматического копирования:
```cmd
cd build\Release
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe temperature_monitor.exe
```

### 5. Сервер недоступен

**Симптом:**
В приложении показывается ошибка подключения.

**Решение:**
- Убедитесь что сервер из lab5 запущен
- Проверьте: `curl http://localhost:8080/api/current`
- Проверьте firewall (разрешите порт 8080)
- Убедитесь что в lab5 используются правильные COM порты

### 6. Компилятор не найден (MinGW)

**Симптом:**
```
Could not find a suitable compiler
```

**Решение:**
- Добавьте MinGW bin в PATH: `C:\mingw64\bin`
- Проверьте: `gcc --version`
- Перезапустите командную строку

### 7. Visual Studio версия не подходит

**Симптом:**
```
Visual Studio 17 2022 could not be found
```

**Решение:**
Попробуйте другие генераторы:
```cmd
cmake .. -G "Visual Studio 16 2019" -A x64
# или
cmake .. -G "Visual Studio 15 2017" -A x64
# или используйте MinGW
cmake .. -G "MinGW Makefiles"
```

### 8. Ошибки линковки с Qt

**Симптом:**
```
unresolved external symbol "public: __cdecl QApplication::QApplication..."
```

**Решение:**
- Убедитесь что используете правильную версию Qt (MSVC vs MinGW)
- Проверьте что Qt Charts установлен
- Пересоберите проект с чистого build:
  ```cmd
  rmdir /s /q build
  mkdir build
  cd build
  cmake ..
  cmake --build .
  ```

## Запуск из Qt Creator

1. **Установить Qt Creator** (идет с Qt Installer)
2. **Открыть проект:**
   - File > Open File or Project
   - Выбрать `lab6/src/CMakeLists.txt`
3. **Настроить Kit:**
   - Projects > Build & Run
   - Выбрать Desktop Qt 6.x.x MSVC2019 64bit
4. **Build:** Ctrl+B
5. **Run:** Ctrl+R

## Создание портативной версии

Для создания standalone версии без установки Qt:

```cmd
cd lab6\src\build\Release
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe --release temperature_monitor.exe
```

Это скопирует все необходимые DLL и плагины в папку с exe.

## Различия Windows vs Unix

### Исполняемый файл
- **Windows:** `temperature_monitor.exe`
- **Unix:** `temperature_monitor`

### Путь сборки
- **Windows:** `build\Release\` (MSVC) или `build\` (MinGW)
- **Unix:** `build/`

### Зависимости
- **Windows:** Qt DLL должны быть в PATH или рядом с exe
- **Unix:** Qt библиотеки загружаются динамически из системы

### Сервер
- **Windows:** Запускается через `run.bat` из lab5
- **Unix:** Запускается через `run.sh` из lab5

## Рекомендуемая конфигурация

Для разработки:
- **Qt:** 6.5.x или 6.6.x
- **Компилятор:** MSVC 2019 (лучшая интеграция с Qt)
- **IDE:** Qt Creator или Visual Studio 2022

Для распространения:
- Собрать с MSVC в Release
- Использовать windeployqt
- Протестировать на чистой системе

## Дополнительные ресурсы

- **Qt Documentation:** https://doc.qt.io/
- **Qt Charts:** https://doc.qt.io/qt-6/qtcharts-index.html
- **CMake Tutorial:** https://cmake.org/cmake/help/latest/guide/tutorial/
- **Qt Creator Manual:** https://doc.qt.io/qtcreator/

## Поддержка

При возникновении проблем:
1. Проверьте что все требования установлены
2. Убедитесь что переменные окружения настроены
3. Попробуйте пересобрать с нуля (удалите папку build)
4. Проверьте что сервер lab5 работает правильно
