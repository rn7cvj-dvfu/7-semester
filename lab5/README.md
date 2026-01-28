# Temperature Monitoring System - Lab 5

Кроссплатформенная система мониторинга температуры с HTTP-сервером и базой данных.

## Возможности

- Чтение данных с виртуального COM порта (симуляция датчика температуры)
- Сохранение измерений в базу данных SQLite
- HTTP API для получения текущей температуры и статистики
- Веб-интерфейс с графиками и таблицами
- Автоматическая очистка старых данных (старше 30 дней)
- Агрегация данных по часам и дням
- Кроссплатформенность (Windows, Linux, macOS)

## Структура проекта

```
src/
├── CMakeLists.txt              # Конфигурация сборки
├── build.sh                    # Скрипт сборки (Unix)
├── run.sh                      # Скрипт запуска (Unix)
├── sensor.cpp                  # Программа симуляции датчика
├── server.cpp                  # Главный сервер
├── date_time/                  # Библиотека работы с датой/временем
├── virtual_com_port/           # Библиотека виртуального COM порта
├── database/                   # Библиотека работы с БД
│   ├── include/database.hpp
│   └── src/database.cpp
├── http_server/                # HTTP сервер
│   ├── include/http_server.hpp
│   └── src/
│       ├── http_server_posix.cpp
│       └── http_server_win.cpp
└── web/                        # Веб-интерфейс
    └── index.html
```

## Требования

### Общие
- CMake >= 3.10
- C++17 совместимый компилятор (GCC, Clang, MSVC)
- SQLite3

### Linux/macOS
```bash
# Ubuntu/Debian
sudo apt-get install cmake g++ libsqlite3-dev

# macOS
brew install cmake sqlite3
```

### Windows
- MinGW-w64 или Visual Studio 2019+
- SQLite3 библиотека

## Сборка

### Linux/macOS

```bash
cd src
chmod +x build.sh run.sh
./build.sh
```

### Windows

```cmd
cd src
mkdir build
cd build
cmake ..
cmake --build .
```

## Запуск

### Linux/macOS

```bash
cd src
./run.sh
```

Скрипт автоматически:
1. Запустит симулятор датчика температуры
2. Запустит сервер с HTTP API
3. Откроет веб-интерфейс на http://localhost:8080

### Windows

```cmd
cd src\build

REM Запуск симулятора датчика (в отдельном окне)
start sensor.exe \\.\COM10 18 28 2000 500

REM Запуск сервера (в отдельном окне)
start server.exe \\.\COM10 data\temperature.db 8080
```

### Ручной запуск

Запуск симулятора датчика:
```bash
./sensor <comPortName> <minTemp> <maxTemp> <interval_ms> <randomShift_ms>

# Пример:
./sensor /tmp/vcom0 18 28 2000 500
```

Запуск сервера:
```bash
./server <comPortName> <databasePath> <httpPort>

# Пример:
./server /tmp/vcom0 ./data/temperature.db 8080
```

## API Endpoints

### GET /
Веб-интерфейс с графиками и таблицами

### GET /api/current
Получить текущую температуру

**Ответ:**
```json
{
  "timestamp": "2026-01-28 14:30:45",
  "temperature": 23
}
```

### GET /api/stats
Получить статистику за период

**Параметры:**
- `start` - Unix timestamp начала периода (необязательно, по умолчанию -24 часа)
- `end` - Unix timestamp конца периода (необязательно, по умолчанию текущее время)
- `period` - тип агрегации: `all`, `hourly`, `daily` (необязательно, по умолчанию `all`)

**Примеры:**

Все измерения за последние 24 часа:
```
GET /api/stats
```

Почасовая статистика за последнюю неделю:
```
GET /api/stats?start=1706198400&end=1706803200&period=hourly
```

Дневная статистика:
```
GET /api/stats?period=daily
```

**Ответ (all):**
```json
[
  {
    "id": 1,
    "timestamp": "2026-01-28 14:30:45",
    "temperature": 23
  },
  ...
]
```

**Ответ (hourly/daily):**
```json
[
  {
    "period_start": "2026-01-28 14:00:00",
    "period_end": "2026-01-28 15:00:00",
    "avg_temperature": 22.5,
    "count": 30
  },
  ...
]
```

## Веб-интерфейс

Откройте http://localhost:8080 в браузере.

**Функции:**
- Отображение текущей температуры в реальном времени
- Интерактивный график температуры (Chart.js)
- Выбор временного периода (1 час, 6 часов, 24 часа, 3 дня, неделя)
- Выбор типа отображения (все измерения, почасовые средние, дневные средние)
- Таблица последних измерений
- Автообновление данных

## База данных

SQLite база данных с таблицей `measurements`:

```sql
CREATE TABLE measurements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    temperature INTEGER NOT NULL
);

CREATE INDEX idx_timestamp ON measurements(timestamp);
```

Данные автоматически удаляются после 30 дней.

## Остановка

Нажмите `Ctrl+C` в терминале, где запущен сервер. Скрипт `run.sh` автоматически остановит все процессы.

## Примечания

- По умолчанию используется виртуальный COM порт `/tmp/vcom0` (Linux/macOS) или `\\.\COM10` (Windows)
- Симулятор генерирует температуру в диапазоне 18-28°C
- Данные отправляются каждые 2 секунды ± 500мс
- HTTP сервер работает на порту 8080
- База данных сохраняется в `./data/temperature.db`

## Лицензия

Учебный проект для курса операционных систем.
