#include "shared_memory.hpp"
#include <cstring>

namespace SharedMemory {

SharedMemoryManager::SharedMemoryManager(const std::string& name)
    : shmHandle_(INVALID_SHM_HANDLE)
    , semHandle_(INVALID_SEM_HANDLE)
    , data_(nullptr)
    , name_(name)
    , isCreator_(false)
{
    // Пытаемся открыть существующую память
    if (!openSharedMemory()) {
        // Если не удалось, создаем новую
        if (createSharedMemory()) {
            isCreator_ = true;
        }
    }

    // Подключаем память
    if (shmHandle_ != INVALID_SHM_HANDLE) {
        mapSharedMemory();
    }

    // Работаем с семафором
    if (!openSemaphore()) {
        createSemaphore();
    }

    // Если все успешно и мы создатели, инициализируем данные
    if (isValid() && isCreator_) {
        data_->counter = 0;
        data_->processCount = 0;
        data_->isMasterActive = false;
        data_->masterPid = 0;
    }

    // Регистрируем подключение процесса
    if (isValid()) {
        lock();
        data_->processCount++;
        unlock();
    }
}

SharedMemoryManager::~SharedMemoryManager() {
    if (isValid()) {
        lock();
        data_->processCount--;
        int count = data_->processCount;
        unlock();

        if (count <= 0) {
            destroySharedMemory();
            destroySemaphore();
        } else {
            unmapSharedMemory();
            closeSharedMemory();
            closeSemaphore();
        }
    }
}

bool SharedMemoryManager::isValid() const {
    return shmHandle_ != INVALID_SHM_HANDLE && 
           semHandle_ != INVALID_SEM_HANDLE && 
           data_ != nullptr;
}

void SharedMemoryManager::lock() {
    if (semHandle_ != INVALID_SEM_HANDLE) {
        WaitForSingleObject(semHandle_, INFINITE);
    }
}

void SharedMemoryManager::unlock() {
    if (semHandle_ != INVALID_SEM_HANDLE) {
        ReleaseSemaphore(semHandle_, 1, NULL);
    }
}

SharedData* SharedMemoryManager::getData() {
    return data_;
}

bool SharedMemoryManager::tryBecomeMaster(int64_t pid) {
    if (!isValid()) return false;
    
    lock();
    bool result = false;
    if (!data_->isMasterActive) {
        data_->isMasterActive = true;
        data_->masterPid = pid;
        result = true;
    }
    unlock();
    
    return result;
}

void SharedMemoryManager::releaseMaster() {
    if (!isValid()) return;
    
    lock();
    data_->isMasterActive = false;
    data_->masterPid = 0;
    unlock();
}

bool SharedMemoryManager::isMaster(int64_t pid) const {
    if (!isValid()) return false;
    return data_->isMasterActive && data_->masterPid == pid;
}

bool SharedMemoryManager::createSharedMemory() {
    std::string mapName = "Local\\" + name_;
    shmHandle_ = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(SharedData),
        mapName.c_str()
    );
    
    return shmHandle_ != INVALID_SHM_HANDLE;
}

bool SharedMemoryManager::openSharedMemory() {
    std::string mapName = "Local\\" + name_;
    shmHandle_ = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        mapName.c_str()
    );
    
    return shmHandle_ != INVALID_SHM_HANDLE;
}

bool SharedMemoryManager::mapSharedMemory() {
    if (shmHandle_ == INVALID_SHM_HANDLE) return false;
    
    data_ = static_cast<SharedData*>(
        MapViewOfFile(shmHandle_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData))
    );
    
    return data_ != nullptr;
}

void SharedMemoryManager::unmapSharedMemory() {
    if (data_ != nullptr) {
        UnmapViewOfFile(data_);
        data_ = nullptr;
    }
}

void SharedMemoryManager::closeSharedMemory() {
    if (shmHandle_ != INVALID_SHM_HANDLE) {
        CloseHandle(shmHandle_);
        shmHandle_ = INVALID_SHM_HANDLE;
    }
}

void SharedMemoryManager::destroySharedMemory() {
    unmapSharedMemory();
    closeSharedMemory();
}

bool SharedMemoryManager::createSemaphore() {
    std::string semName = name_ + "_sem";
    semHandle_ = CreateSemaphoreA(
        NULL,
        1,  // Начальное значение
        1,  // Максимальное значение
        semName.c_str()
    );
    
    return semHandle_ != INVALID_SEM_HANDLE;
}

bool SharedMemoryManager::openSemaphore() {
    std::string semName = name_ + "_sem";
    semHandle_ = OpenSemaphoreA(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        semName.c_str()
    );
    
    return semHandle_ != INVALID_SEM_HANDLE;
}

void SharedMemoryManager::closeSemaphore() {
    if (semHandle_ != INVALID_SEM_HANDLE) {
        CloseHandle(semHandle_);
        semHandle_ = INVALID_SEM_HANDLE;
    }
}

void SharedMemoryManager::destroySemaphore() {
    closeSemaphore();
}

} // namespace SharedMemory
