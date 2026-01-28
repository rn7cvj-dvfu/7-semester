# SQLite3 для lab5

## Установка SQLite3

### Вариант 1: Скачать с официального сайта (Windows)

1. Перейдите на https://www.sqlite.org/download.html
2. Скачайте:
   - **sqlite-amalgamation-*.zip** - для заголовочных файлов
   - **sqlite-dll-win-x64-*.zip** - для DLL и LIB файлов

3. Распакуйте файлы:
   - `sqlite3.h` → поместите в `include/`
   - `sqlite3ext.h` → поместите в `include/`
   - `sqlite3.lib` → поместите в `lib/`
   - `sqlite3.dll` → скопируйте в `../../build/` или в системную папку

### Вариант 2: Использовать vcpkg

```powershell
# Установить vcpkg (если не установлен)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Установить SQLite3
.\vcpkg install sqlite3:x64-windows

# Интегрировать с Visual Studio
.\vcpkg integrate install
```

### Вариант 3: Использовать пакетный менеджер

```powershell
# Через chocolatey
choco install sqlite

# Или через scoop
scoop install sqlite
```

## Проверка установки

После установки файлов, структура должна быть:

```
external/sqlite3/
  include/
    sqlite3.h
    sqlite3ext.h
  lib/
    sqlite3.lib
```

Затем пересоберите проект:

```bash
cd ../../build
cmake ..
cmake --build . --config Release
```
