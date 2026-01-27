# Сервер сбора данных с датчиков (Lab5)

Веб-сервер на Python с PostgreSQL для сбора данных с последовательного порта (из lab4) и предоставления API для получения записей.

## Возможности

- **Автоматический сбор данных** с последовательного COM-порта
- **Хранение в PostgreSQL** с индексацией по времени
- **REST API** для получения данных в различных диапазонах
- **Docker поддержка** для простого развертывания
- **Автоматическая документация** API (Swagger UI)

## Структура проекта

```
lab5/
├── docker-compose.yml          # Docker Compose конфигурация
├── Dockerfile                  # Docker образ приложения
├── .gitignore                 # Игнорируемые файлы
├── README.md                  # Документация
└── src/
    ├── main.py                # Главное FastAPI приложение
    ├── config.py              # Конфигурация
    ├── database.py            # Работа с PostgreSQL
    ├── serial_reader.py       # Чтение с COM-порта
    ├── requirements.txt       # Python зависимости
    └── .env.example          # Пример переменных окружения
```

## Требования

### Вариант 1: С Docker (рекомендуется)
- Docker Desktop (Windows) или Docker + Docker Compose (Linux)

### Вариант 2: Без Docker
- Python 3.11+
- PostgreSQL 14+
- COM-порт с данными из lab4

## Быстрый старт

### С Docker

1. **Скопируйте файл окружения:**
```powershell
Copy-Item src\.env.example src\.env
```

2. **Отредактируйте `.env` файл:**
```env
SERIAL_PORT=COM3          # Ваш COM-порт
```

3. **Запустите сервисы:**
```powershell
docker-compose up -d
```

4. **Проверьте статус:**
```powershell
docker-compose ps
```

### Без Docker

1. **Создайте виртуальное окружение:**
```powershell
cd src
python -m venv venv
.\venv\Scripts\Activate.ps1
```

2. **Установите зависимости:**
```powershell
pip install -r requirements.txt
```

3. **Настройте PostgreSQL:**
```sql
CREATE DATABASE sensor_data;
CREATE USER postgres WITH PASSWORD 'postgres';
GRANT ALL PRIVILEGES ON DATABASE sensor_data TO postgres;
```

4. **Настройте переменные окружения:**
```powershell
Copy-Item .env.example .env
# Отредактируйте .env файл
```

5. **Запустите сервер:**
```powershell
python main.py
```

## API Endpoints

Сервер запускается на `http://localhost:8000`

### Документация
- **Swagger UI:** http://localhost:8000/docs
- **ReDoc:** http://localhost:8000/redoc

### Основные endpoints

#### `GET /` - Корневой endpoint
Проверка работоспособности API.

#### `GET /health` - Статус сервера
Возвращает статус здоровья сервиса.

#### `GET /readings/latest` - Последние записи
Получение последних N записей с датчика.

**Параметры:**
- `limit` (int, опционально): Количество записей (по умолчанию 100, макс 1000)

**Пример:**
```bash
curl "http://localhost:8000/readings/latest?limit=50"
```

#### `GET /readings/by-timestamp` - Записи по Unix timestamp
Получение записей в диапазоне временных меток.

**Параметры:**
- `start` (int, обязательно): Начальная временная метка
- `end` (int, обязательно): Конечная временная метка
- `limit` (int, опционально): Макс записей (по умолчанию 1000, макс 10000)

**Пример:**
```bash
curl "http://localhost:8000/readings/by-timestamp?start=1737878400&end=1737964800&limit=100"
```

#### `GET /readings/by-datetime` - Записи по дате и времени
Получение записей в диапазоне дат (по времени получения сервером).

**Параметры:**
- `start` (string, обязательно): Начальная дата (ISO 8601)
- `end` (string, обязательно): Конечная дата (ISO 8601)
- `limit` (int, опционально): Макс записей (по умолчанию 1000, макс 10000)

**Пример:**
```bash
curl "http://localhost:8000/readings/by-datetime?start=2026-01-26T00:00:00&end=2026-01-26T23:59:59"
```

**Пример с PowerShell:**
```powershell
$start = "2026-01-26T00:00:00"
$end = "2026-01-26T23:59:59"
Invoke-RestMethod "http://localhost:8000/readings/by-datetime?start=$start&end=$end"
```

## Формат данных

### SensorReading
```json
{
  "id": 1,
  "sensor_value": 42,
  "timestamp": 1737878400,
  "received_at": "2026-01-26T10:00:00",
  "created_at": "2026-01-26T10:00:00"
}
```

**Поля:**
- `id`: Уникальный идентификатор записи
- `sensor_value`: Значение с датчика
- `timestamp`: Unix timestamp от датчика
- `received_at`: Время получения сервером
- `created_at`: Время создания записи в БД

## Интеграция с Lab4

Сервер читает данные с COM-порта, на который отправляет данные `sensor` из lab4.

**Формат данных от sensor.cpp:**
```
<sensor_value>|<timestamp>\n
```

**Пример:**
```
42|1737878400
```

### Запуск с Lab4

1. **Запустите веб-сервер (lab5):**
```powershell
cd lab5
docker-compose up -d
```

2. **Запустите sensor из lab4:**
```powershell
cd lab4/src/build
.\sensor.exe COM3 0 100 1000 100
```

Сервер автоматически начнет сохранять данные в PostgreSQL.

## Управление

### Просмотр логов
```powershell
# Все сервисы
docker-compose logs -f

# Только веб-сервер
docker-compose logs -f web

# Только база данных
docker-compose logs -f postgres
```

### Остановка сервисов
```powershell
docker-compose down
```

### Остановка с удалением данных
```powershell
docker-compose down -v
```

### Перезапуск
```powershell
docker-compose restart
```

## Подключение к базе данных

### С Docker
```powershell
docker-compose exec postgres psql -U postgres -d sensor_data
```

### Без Docker
```powershell
psql -h localhost -U postgres -d sensor_data
```

### Полезные SQL запросы

```sql
-- Количество записей
SELECT COUNT(*) FROM sensor_readings;

-- Последние 10 записей
SELECT * FROM sensor_readings ORDER BY received_at DESC LIMIT 10;

-- Средние значения по часам
SELECT 
    date_trunc('hour', received_at) as hour,
    AVG(sensor_value) as avg_value,
    COUNT(*) as count
FROM sensor_readings
GROUP BY hour
ORDER BY hour DESC;

-- Очистка всех данных
TRUNCATE TABLE sensor_readings RESTART IDENTITY;
```

## Переменные окружения

| Переменная | Описание | По умолчанию |
|------------|----------|--------------|
| `POSTGRES_HOST` | Хост PostgreSQL | localhost |
| `POSTGRES_PORT` | Порт PostgreSQL | 5432 |
| `POSTGRES_DB` | Имя базы данных | sensor_data |
| `POSTGRES_USER` | Пользователь БД | postgres |
| `POSTGRES_PASSWORD` | Пароль БД | postgres |
| `SERIAL_PORT` | COM-порт | COM0 |
| `SERIAL_BAUDRATE` | Скорость порта | 9600 |
| `SERIAL_TIMEOUT` | Таймаут чтения | 1 |
| `SERVER_HOST` | Адрес сервера | 0.0.0.0 |
| `SERVER_PORT` | Порт сервера | 8000 |

## Решение проблем

### Ошибка подключения к COM-порту

**Проблема:** `Failed to open serial port`

**Решение:**
1. Проверьте, что порт существует: `mode` (Windows) или `ls /dev/tty*` (Linux)
2. Убедитесь, что порт не используется другим приложением
3. Проверьте права доступа к порту
4. На Windows: убедитесь, что в Docker Desktop включен доступ к устройствам

### Ошибка подключения к базе данных

**Проблема:** `Error creating database connection pool`

**Решение:**
1. Проверьте, что PostgreSQL запущен: `docker-compose ps`
2. Проверьте переменные окружения в `.env`
3. Подождите несколько секунд после запуска для инициализации БД

### Порт 8000 уже занят

**Решение:**
```powershell
# Измените порт в docker-compose.yml
ports:
  - "8080:8000"  # Используйте 8080 вместо 8000
```

## Тестирование

### Тест с curl (PowerShell)
```powershell
# Проверка здоровья
Invoke-RestMethod http://localhost:8000/health

# Последние записи
Invoke-RestMethod http://localhost:8000/readings/latest?limit=10

# Записи по timestamp (последние 24 часа)
$end = [int][double]::Parse((Get-Date -UFormat %s))
$start = $end - 86400
Invoke-RestMethod "http://localhost:8000/readings/by-timestamp?start=$start&end=$end"
```

### Тест с Python
```python
import requests
from datetime import datetime, timedelta

base_url = "http://localhost:8000"

# Последние записи
response = requests.get(f"{base_url}/readings/latest", params={"limit": 10})
print(response.json())

# По дате
end = datetime.now()
start = end - timedelta(hours=24)
response = requests.get(
    f"{base_url}/readings/by-datetime",
    params={
        "start": start.isoformat(),
        "end": end.isoformat(),
        "limit": 100
    }
)
print(response.json())
```

## Лицензия

Учебный проект для ДВФУ, OS, 7 семестр.
