#include <string>
#include <functional>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__unix__)
    #include <termios.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <pty.h>
    #include <sys/select.h>
#endif

namespace VirtualCOM {

    /**
     * @brief Результат операции с виртуальным COM портом
     */
    struct ComResult {
        bool success;
        std::string error;
    };

    /**
     * @brief Класс для работы с виртуальным COM портом
     */
    class VirtualComPort {
    public:
        
        /**
         * @brief Конструктор по умолчанию (порт не открывается)
         */
        VirtualComPort();
        
        /**
         * @brief Конструктор с автоматическим открытием существующего порта
         * @param portName Имя порта
         * @param baudRate Скорость передачи данных
         * @throw std::runtime_error если не удалось открыть порт
         */
        VirtualComPort(const std::string& portName, int baudRate = 9600);
        
        /**
         * @brief Запись данных в порт
         * @param data Данные для записи
         * @return Количество записанных байт
         */
        int write(const std::string& data);

        /**
         * @brief Чтение данных из порта
         * @param maxBytes Максимальное количество байт для чтения
         * @param timeoutMs Таймаут в миллисекундах
         * @return Прочитанные данные
         */
        std::string read(size_t maxBytes = 1024, int timeoutMs = 1000);

        /**
         * @brief Проверка, открыт ли порт
         */
        bool isOpen() const;

        
        /**
         * @brief Закрытие COM порта
         */
        void close();

        ~VirtualComPort();


    };

} 

