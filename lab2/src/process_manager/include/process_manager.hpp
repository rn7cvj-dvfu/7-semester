#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Пространство имен для управления фоновыми процессами
 */
namespace ProcessManager {

#ifdef _WIN32
    using ProcessHandle = HANDLE;
#elif defined(__unix__)
    using ProcessHandle = pid_t;
#endif

    /**
     * @brief Результат запуска процесса
     * @param success Успешность запуска
     * @param handle Дескриптор процесса
     * @param error Описание ошибки (если есть)
     */
    struct LaunchResult {
        bool success;          
        ProcessManager::ProcessHandle handle;   
        std::string error;      
    };

    /**
     * @brief Результат ожидания завершения процесса
     * @param success Успешность ожидания
     * @param exitCode Код возврата процесса
     * @param error Описание ошибки (если есть)
     */
    struct WaitResult {
        bool success;          
        int exit_code;           
        std::string error;      
    };

    /**
     * @brief Запускает процесс в фоновом режиме
     * 
     * @param command Команда для выполнения (путь к исполняемому файлу)
     * @param args Аргументы командной строки
     * @param silent Тихий запуск без окна (только для Windows)
     * @return LaunchResult Результат запуска
     */
    LaunchResult launchProcess(
        const std::string& command, 
        const std::vector<std::string>& args = {},
        bool silent = false
    );

    /**
     * @brief Запускает терминал с командой
     * 
     * @param args Аргументы командной строки для терминала
     * @return LaunchResult Результат запуска терминала
     */
    LaunchResult launchTerminal(const std::vector<std::string>& args = {});

    /**
     * @brief Ожидает завершения процесса и получает код возврата
     * 
     * @param handle Дескриптор процесса
     * @param timeout_ms Таймаут в миллисекундах (0 - бесконечно)
     * @return WaitResult Результат ожидания с кодом возврата
     */
    WaitResult waitForProcess(
        ProcessManager::ProcessHandle handle, 
        unsigned int timeout_ms = 0
    );

    /**
     * @brief Ожидает закрытия окна терминала и получает код возврата
     * 
     * @param handle Дескриптор процесса терминала
     * @param timeout_ms Таймаут в миллисекундах (0 - бесконечно)
     * @return WaitResult Результат ожидания с кодом возврата
     */
    WaitResult waitForTerminal(
        ProcessManager::ProcessHandle handle, 
        unsigned int timeout_ms = 0
    );

    /**
     * @brief Проверяет, запущен ли процесс
     * 
     * @param handle Дескриптор процесса
     * @return true Если процесс еще работает
     * @return false Если процесс завершился
     */
    bool isProcessRunning(ProcessManager::ProcessHandle handle);

    /**
     * @brief Принудительно завершает процесс
     * 
     * @param handle Дескриптор процесса
     * @return true Если процесс был завершен успешно
     * @return false В случае ошибки
     */
    bool terminateProcess(ProcessManager::ProcessHandle handle);

    /**
     * @brief Освобождает ресурсы дескриптора процесса
     * 
     * @param handle Дескриптор процесса
     */
    void closeHandle(ProcessManager::ProcessHandle handle);

    /**
     * @brief Получает идентификатор текущего процесса
     */
    int getProcessID();

    namespace internal {
        
#ifdef _WIN32
        std::wstring buildCommandLine(
            const std::string& command, 
            const std::vector<std::string>& args
        );

        std::wstring stringToWString(const std::string& str);

#elif defined(__unix__)
        char** buildArgv(
            const std::string& command, 
            const std::vector<std::string>& args
        );
                        
        void freeArgv(char** argv);
#endif
    } 

} 

