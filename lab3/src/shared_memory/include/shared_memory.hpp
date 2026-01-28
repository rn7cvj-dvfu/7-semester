#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#include <string>
#include <cstdint>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__unix__)
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <semaphore.h>
#endif

/**
 * @brief Пространство имен для работы с разделяемой памятью
 */
namespace SharedMemory {

#ifdef _WIN32
    using ShmHandle = HANDLE;
    using SemHandle = HANDLE;
#elif defined(__unix__)
    using ShmHandle = int;
    using SemHandle = sem_t*;
#endif

    /**
     * @brief Структура данных в разделяемой памяти
     */
    struct SharedData {
        int64_t counter;
        int process_count;
        bool is_master_active;
        int64_t master_pid;
    };

    /**
     * @brief Результат операции с разделяемой памятью
     */
    struct ShmResult {
        bool success;
        std::string error;
    };

    /**
     * @brief Класс для работы с разделяемой памятью
     */
    class SharedMemoryManager {
    public:
        /**
         * @brief Конструктор
         * @param name Имя объекта разделяемой памяти
         */
        SharedMemoryManager(const std::string& name);
        
        /**
         * @brief Деструктор
         */
        ~SharedMemoryManager();

        /**
         * @brief Проверка валидности объекта
         */
        bool isValid() const;

        /**
         * @brief Блокировка доступа к данным
         */
        void lock();

        /**
         * @brief Разблокировка доступа к данным
         */
        void unlock();

        /**
         * @brief Получение указателя на данные
         */
        SharedData* getData();

        /**
         * @brief Попытка стать мастер-процессом
         * @return true если успешно стал мастером
         */
        bool tryBecomeMaster(int64_t pid);

        /**
         * @brief Освобождение роли мастер-процесса
         */
        void releaseMaster();

        /**
         * @brief Проверка, является ли текущий процесс мастером
         */
        bool isMaster(int64_t pid) const;

    private:
        ShmHandle shm_handle_;
        SemHandle sem_handle_;
        SharedData* data_;
        std::string name_;
        bool is_creator_;

        bool createSharedMemory();
        bool openSharedMemory();
        bool mapSharedMemory();
        void unmapSharedMemory();
        void closeSharedMemory();
        void destroySharedMemory();

        bool createSemaphore();
        bool openSemaphore();
        void closeSemaphore();
        void destroySemaphore();

#ifdef _WIN32
        static constexpr HANDLE INVALID_SHM_HANDLE = NULL;
        static constexpr HANDLE INVALID_SEM_HANDLE = NULL;
#elif defined(__unix__)
        static constexpr int INVALID_SHM_HANDLE = -1;
        static constexpr sem_t* INVALID_SEM_HANDLE = SEM_FAILED;
#endif
    };

} // namespace SharedMemory

#endif // SHARED_MEMORY_HPP
