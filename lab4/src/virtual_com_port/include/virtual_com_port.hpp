#include <string>
#include <functional>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__)
#include <termios.h>
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
         * @param port_name Имя порта
         * @param baud_rate Скорость передачи данных
         * @throw std::runtime_error если не удалось открыть порт
         */
        VirtualComPort(const std::string& port_name, int baud_rate = 9600);
        
        /**
         * @brief Запись данных в порт
         * @param data Данные для записи
         * @return Количество записанных байт
         */
        int write(const std::string& data);

        /**
         * @brief Чтение данных из порта
         * @param max_bytes Максимальное количество байт для чтения
         * @param timeout_ms Таймаут в миллисекундах
         * @return Прочитанные данные
         */
        std::string read(size_t max_bytes = 1024, int timeout_ms = 1000);

        /**
         * @brief Проверка, открыт ли порт
         */
        bool isOpen() const;

        
        /**
         * @brief Закрытие COM порта
         */
        void close();

        ~VirtualComPort();

    private:
#ifdef _WIN32
        HANDLE handle_;
        std::string port_name_;
#elif defined(__unix__)
        int fd_;
        std::string port_name_;
        struct termios old_termios_;
#endif
        bool is_open_;
        int baud_rate_;

        bool configurePort();
        
#ifdef _WIN32
        bool configureWindowsPort();
#elif defined(__unix__)
        bool configureUnixPort();
#endif
    };

} 

