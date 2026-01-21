#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include <string>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/types.h>
#endif

/**
 * @brief Пространство имен для управления фоновыми процессами
 */
namespace ProcessManager {

#ifdef _WIN32
    using ProcessHandle = HANDLE;
#else
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
        ProcessHandle handle;   
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
        int exitCode;           
        std::string error;      
    };

    /**
     * @brief Запускает процесс в фоновом режиме
     * 
     * @param command Команда для выполнения (путь к исполняемому файлу)
     * @param args Аргументы командной строки
     * @return LaunchResult Результат запуска
     */
    LaunchResult launchProcess(const std::string& command, 
                               const std::vector<std::string>& args = {});

    /**
     * @brief Ожидает завершения процесса и получает код возврата
     * 
     * @param handle Дескриптор процесса
     * @param timeoutMs Таймаут в миллисекундах (0 - бесконечно)
     * @return WaitResult Результат ожидания с кодом возврата
     */
    WaitResult waitForProcess(ProcessHandle handle, unsigned int timeoutMs = 0);

    /**
     * @brief Проверяет, запущен ли процесс
     * 
     * @param handle Дескриптор процесса
     * @return true Если процесс еще работает
     * @return false Если процесс завершился
     */
    bool isProcessRunning(ProcessHandle handle);

    /**
     * @brief Принудительно завершает процесс
     * 
     * @param handle Дескриптор процесса
     * @return true Если процесс был завершен успешно
     * @return false В случае ошибки
     */
    bool terminateProcess(ProcessHandle handle);

    /**
     * @brief Освобождает ресурсы дескриптора процесса
     * 
     * @param handle Дескриптор процесса
     */
    void closeHandle(ProcessHandle handle);

  
    namespace internal {
#ifdef _WIN32
       
        std::wstring buildCommandLine(const std::string& command, 
                                      const std::vector<std::string>& args);
        std::wstring stringToWString(const std::string& str);
#else
        char** buildArgv(const std::string& command, 
                        const std::vector<std::string>& args);
        void freeArgv(char** argv);
#endif
    } 

} 

#endif 
