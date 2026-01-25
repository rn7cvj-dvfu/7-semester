# Лабораторная работа 4: Виртуальный COM порт

Кроссплатформенная программа для создания и работы с виртуальными COM портами.

## Возможности

- ✅ Создание виртуального COM порта
- ✅ Открытие существующего COM порта
- ✅ Чтение и запись данных
- ✅ Настройка скорости передачи (baud rate)
- ✅ Эхо-сервер для тестирования
- ✅ Кроссплатформенность (Windows, Linux, macOS)

## Архитектура

### Windows
На Windows программа использует WinAPI для работы с COM портами:
- `CreateFile` - открытие порта
- `ReadFile/WriteFile` - чтение/запись данных
- `DCB` - конфигурация порта

**Примечание**: Для создания виртуальной пары COM портов в Windows требуется драйвер (например, [com0com](https://sourceforge.net/projects/com0com/)). Программа может работать с существующими портами.

### Linux/Unix
На Linux/macOS используются pseudo-terminals (PTY):
- `openpty()` - создание виртуального терминала
- `termios` - конфигурация порта
- `read/write` - обмен данными

PTY создает пару master-slave портов, где:
- Master используется программой
- Slave (`/dev/pts/X`) используется другими программами

## Сборка

### Linux/macOS
```bash
cd src
mkdir -p build && cd build
cmake ..
make
```

### Windows (MinGW)
```bash
cd src
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### Windows (Visual Studio)
```bash
cd src
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

## Использование

### Создание виртуального порта
```bash
# Linux/macOS - создает /dev/pts/X
./vcom_port create [baudRate]

# Пример
./vcom_port create 9600
```

Программа создаст виртуальный COM порт и выведет его имя. На Linux это будет что-то вроде `/dev/pts/3`.

### Тестирование порта
```bash
# Отправка тестовых данных на порт
./vcom_port test <portName>

# Примеры
./vcom_port test COM3          # Windows
./vcom_port test /dev/pts/3    # Linux
```

### Эхо-сервер
```bash
# Запуск эхо-сервера (отправляет обратно все полученные данные)
./vcom_port echo <portName>

# Примеры
./vcom_port echo COM3          # Windows
./vcom_port echo /dev/pts/3    # Linux
```

## Примеры использования

### Пример 1: Создание и тестирование (Linux)

**Терминал 1** - создаем порт:
```bash
./vcom_port create 9600
# Вывод:
# Virtual COM port created successfully!
# Master (this process): master
# Slave (use this in other programs): /dev/pts/3
```

**Терминал 2** - подключаемся к slave порту:
```bash
# Используем minicom или другую программу
minicom -D /dev/pts/3

# Или нашу программу
./vcom_port test /dev/pts/3
```

### Пример 2: Двусторонняя связь (Linux)

**Терминал 1**:
```bash
./vcom_port create 9600
# Записываем имя slave порта, например /dev/pts/5
```

**Терминал 2**:
```bash
# Отправка данных в порт
echo "Hello, COM port!" > /dev/pts/5

# Или чтение из порта
cat /dev/pts/5
```

### Пример 3: Эхо-тест (Windows)

**Терминал 1**:
```bash
vcom_port.exe echo COM3
```

**Терминал 2**:
```bash
vcom_port.exe test COM3
```

## Технические детали

### Конфигурация порта
- **Baud rate**: 9600, 19200, 38400, 57600, 115200 (настраивается)
- **Data bits**: 8
- **Stop bits**: 1
- **Parity**: None
- **Flow control**: None

### Структура проекта
```
lab4/
├── src/
│   ├── main.cpp                                  # Основная программа
│   ├── CMakeLists.txt                            # Конфигурация сборки
│   └── virtual_com_port/
│       ├── include/
│       │   └── virtual_com_port.hpp              # Заголовочный файл
│       └── src/
│           ├── virtual_com_port_win.cpp          # Реализация для Windows
│           └── virtual_com_port_posix.cpp        # Реализация для POSIX
└── README.md
```

## API

### Класс VirtualComPort

```cpp
// Создание виртуального порта
ComResult create(const std::string& portName = "", int baudRate = 9600);

// Открытие существующего порта
ComResult open(const std::string& portName, int baudRate = 9600);

// Закрытие порта
void close();

// Проверка состояния
bool isOpen() const;

// Запись данных
int write(const std::string& data);

// Чтение данных
std::string read(size_t maxBytes = 1024, int timeoutMs = 1000);

// Получение имен портов
std::string getPortName() const;
std::string getSlavePortName() const;  // Только для Unix
```

## Дополнительные инструменты

### Linux
- `socat` - универсальный инструмент для создания виртуальных портов
  ```bash
  socat -d -d pty,raw,echo=0 pty,raw,echo=0
  ```
- `minicom` - терминальная программа для работы с COM портами
- `screen` - альтернатива minicom

### Windows
- [com0com](https://sourceforge.net/projects/com0com/) - драйвер виртуальных COM портов
- [PuTTY](https://www.putty.org/) - терминальная программа
- Встроенный Device Manager для управления портами

## Устранение неполадок

### Linux: Permission denied
```bash
# Добавьте пользователя в группу dialout
sudo usermod -a -G dialout $USER
# Перелогиньтесь

# Или измените права для конкретного порта
sudo chmod 666 /dev/ttyUSB0
```

### Windows: Порт не найден
- Убедитесь, что порт существует (Device Manager)
- Для виртуальных портов установите com0com
- Проверьте, что порт не используется другой программой

### Общие проблемы
- **Timeout при чтении**: Увеличьте таймаут или проверьте, что данные действительно отправляются
- **Неправильная скорость**: Убедитесь, что обе стороны используют одинаковый baud rate
- **Мусорные данные**: Проверьте настройки четности и стоп-битов

## Лицензия

MIT License
