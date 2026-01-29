# Лабораторная работа №6 - Temperature Monitor GUI

GUI приложение на C++/Qt для визуализации данных с сервера мониторинга температуры.

## Описание

Desktop приложение с графическим интерфейсом для отображения:
- Текущей температуры в реальном времени
- Интерактивного графика температуры (Qt Charts)
- Таблицы с историей измерений
- Различных периодов и типов агрегации данных

## Возможности

✅ **Текущая температура** - крупное отображение с timestamp  
✅ **График** - интерактивный график на Qt Charts с масштабированием  
✅ **Таблица** - последние 20 измерений  
✅ **Настройки периода** - час, 6 часов, 24 часа, 3 дня  
✅ **Типы отображения** - все измерения, среднее по часам, среднее по дням  
✅ **Автообновление** - каждые 5 секунд  
✅ **Кроссплатформенность** - Windows, Linux, macOS

## Требования

### Общие
- **CMake** >= 3.16
- **C++17** компилятор
- **Qt6** или **Qt5** (Core, Widgets, Network, Charts)

### Установка Qt

#### macOS
```bash
brew install qt
```

#### Ubuntu/Debian
```bash
sudo apt-get install qt6-base-dev qt6-charts-dev libqt6charts6-dev
# или для Qt5:
sudo apt-get install qtbase5-dev qtcharts5-dev libqt5charts5-dev
```

#### Windows
Скачайте Qt Online Installer: https://www.qt.io/download-qt-installer

Установите компоненты:
- Qt 6.x (или 5.15.x)
- Qt Charts
- MinGW или MSVC компилятор

**Подробная инструкция:** См. [WINDOWS_SETUP.md](WINDOWS_SETUP.md)

## Быстрый старт

### 1. Убедитесь что сервер запущен

Сервер из lab5 должен работать на `http://localhost:8080`:

#### Linux/macOS
```bash
cd ../lab5/src
./run.sh
```

#### Windows
```cmd
cd ..\lab5\src
run.bat
```

### 2. Сборка GUI приложения

#### Linux/macOS

```bash
cd lab6/src
chmod +x build.sh run.sh
./build.sh
```

#### Windows

```cmd
cd lab6\src
build.bat
```

**Примечание для Windows:** См. [WINDOWS_SETUP.md](WINDOWS_SETUP.md) для подробных инструкций по установке Qt и настройке окружения.

### 3. Запуск приложения

#### Linux/macOS

```bash
./run.sh
```

#### Windows

```cmd
run.bat
```

## Использование

### Главное окно

1. **Текущая температура** - отображается вверху большими цифрами
2. **Панель управления**:
   - Выбор периода времени
   - Выбор типа отображения данных
   - Кнопка ручного обновления
   - Переключатель автообновления
3. **График** - интерактивный график Qt Charts
4. **Таблица** - детальные данные

### Горячие клавиши

- `F5` - Обновить данные
- `Ctrl+Q` - Выход

### Функции графика

- **Масштабирование** - колесо мыши
- **Перемещение** - перетаскивание мышью
- **Сброс** - двойной клик

## Структура проекта

```
lab6/
├── README.md
└── src/
    ├── CMakeLists.txt
    ├── build.sh
    ├── run.sh
    ├── main.cpp              # Точка входа
    ├── mainwindow.h          # Главное окно
    ├── mainwindow.cpp
    ├── apihandler.h          # HTTP API клиент
    └── apihandler.cpp
```

## Архитектура

### Классы

#### MainWindow
- Главное окно приложения
- Управление UI компонентами
- Обработка пользовательского ввода

#### ApiHandler
- HTTP клиент для общения с сервером
- Парсинг JSON ответов
- Преобразование данных в Qt структуры

### Структуры данных

```cpp
struct TemperatureMeasurement {
    QDateTime timestamp;
    double temperature;
};

struct AggregatedTemperature {
    QDateTime periodStart;
    QDateTime periodEnd;
    double avgTemperature;
    int count;
};
```

## API Endpoints

Приложение использует следующие endpoints сервера:

- `GET /api/current` - Текущая температура
- `GET /api/stats?start=<time>&end=<time>&period=<type>` - Статистика

## Решение проблем

### Qt не найден

**macOS:**
```bash
brew install qt
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt/lib/cmake"
```

**Linux:**
```bash
sudo apt-get install qt6-base-dev qt6-charts-dev
# или укажите путь:
export CMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt6"
**Windows:**
```cmd
set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
```

См. [WINDOWS_SETUP.md](WINDOWS_SETUP.md) для подробного руководства.

### Сервер недоступен

Убедитесь что:
1. Сервер из lab5 запущен
2. Работает на порту 8080
3. Доступен по адресу http://localhost:8080

Проверка:

**Unix:**
```bash
curl http://localhost:8080/api/current
```

**Windows:**
```cmd
curl http://localhost:8080/api/current
# или
powershell -Command "Invoke-WebRequest -Uri http://localhost:8080/api/current"
```

### Qt Charts не найден

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install libqt6charts6-dev

# или для Qt5
sudo apt-get install libqt5charts5-dev
```

**Windows:**
- Запустите Qt Maintenance Tool
- Добавьте компонент "Qt Charts"
- См. [WINDOWS_SETUP.md](WINDOWS_SETUP.md) для деталей

### Ошибка сборки на macOS

Если CMake не находит Qt:
```bash
export Qt6_DIR="/opt/homebrew/opt/qt/lib/cmake/Qt6"
cmake ..
```

### Проблемы на Windows

Для решения проблем на Windows см. детальное руководство: [WINDOWS_SETUP.md](WINDOWS_SETUP.md)

Основные проблемы:
- Qt DLL не найдены → добавьте Qt bin в PATH
- Компилятор не найден → установите MinGW или Visual Studio
- CMake ошибки → проверьте переменные окруженияort Qt6_DIR="/opt/homebrew/opt/qt/lib/cmake/Qt6"
cmake ..
```

## Особенности реализации

### Кроссплатформенность
- Использует Qt Framework для полной кроссплатформенности
- Один код работает на Windows, Linux, macOS
- Нативный look & feel на каждой платформе

### Производительность
- Асинхронные HTTP запросы (QNetworkAccessManager)
- Эффективная отрисовка графиков (Qt Charts)
- Минимальное потребление ресурсов

### UI/UX
- Минималистичный дизайн
- Интуитивный интерфейс
- Адаптивная компоновка
- Темная/светлая тема (следует за системой)

## Зависимости

- **Qt6** (или Qt5) >= 5.15
  - Qt Core
  - Qt Widgets  
  - Qt Network
  - Qt Charts
- **CMake** >= 3.16
- **C++17** компилятор

## Сборка с Qt Creator

1. Откройте `CMakeLists.txt` в Qt Creator
2. Настройте Kit (Desktop Qt 6.x)
3. Нажмите Build (Ctrl+B)
4. Нажмите Run (Ctrl+R)

## Лицензия

Учебный проект
