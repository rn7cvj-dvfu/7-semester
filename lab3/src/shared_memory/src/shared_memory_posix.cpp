#include "shared_memory.hpp"
#include <cstring>

namespace SharedMemory {

SharedMemoryManager::SharedMemoryManager(const std::string& name)
    : shmHandle_(INVALID_SHM_HANDLE)
    , semHandle_(INVALID_SEM_HANDLE)
    , data_(nullptr)
    , name_("/" + name)
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
    std::string semName = name_ + "_sem";
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
        sem_wait(semHandle_);
    }
}

void SharedMemoryManager::unlock() {
    if (semHandle_ != INVALID_SEM_HANDLE) {
        sem_post(semHandle_);
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
    shmHandle_ = shm_open(name_.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    
    if (shmHandle_ != INVALID_SHM_HANDLE) {
        ftruncate(shmHandle_, sizeof(SharedData));
        return true;
    }
    
    return false;
}

bool SharedMemoryManager::openSharedMemory() {
    shmHandle_ = shm_open(name_.c_str(), O_RDWR, 0644);
    return shmHandle_ != INVALID_SHM_HANDLE;
}

bool SharedMemoryManager::mapSharedMemory() {
    if (shmHandle_ == INVALID_SHM_HANDLE) return false;
    
    void* addr = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, 
                      MAP_SHARED, shmHandle_, 0);
    
    if (addr == MAP_FAILED) {
        data_ = nullptr;
        return false;
    }
    
    data_ = static_cast<SharedData*>(addr);
    return true;
}

void SharedMemoryManager::unmapSharedMemory() {
    if (data_ != nullptr) {
        munmap(data_, sizeof(SharedData));
        data_ = nullptr;
    }
}

void SharedMemoryManager::closeSharedMemory() {
    if (shmHandle_ != INVALID_SHM_HANDLE) {
        close(shmHandle_);
        shmHandle_ = INVALID_SHM_HANDLE;
    }
}

void SharedMemoryManager::destroySharedMemory() {
    unmapSharedMemory();
    
    if (shmHandle_ != INVALID_SHM_HANDLE) {
        close(shmHandle_);
        shm_unlink(name_.c_str());
        shmHandle_ = INVALID_SHM_HANDLE;
    }
}

bool SharedMemoryManager::createSemaphore() {
    std::string semName = name_ + "_sem";
    semHandle_ = sem_open(semName.c_str(), O_CREAT | O_EXCL, 0644, 1);
    
    return semHandle_ != INVALID_SEM_HANDLE;
}

bool SharedMemoryManager::openSemaphore() {
    std::string semName = name_ + "_sem";
    semHandle_ = sem_open(semName.c_str(), 0);
    
    return semHandle_ != INVALID_SEM_HANDLE;
}

void SharedMemoryManager::closeSemaphore() {
    if (semHandle_ != INVALID_SEM_HANDLE) {
        sem_close(semHandle_);
        semHandle_ = INVALID_SEM_HANDLE;
    }
}

void SharedMemoryManager::destroySemaphore() {
    if (semHandle_ != INVALID_SEM_HANDLE) {
        std::string semName = name_ + "_sem";
        sem_close(semHandle_);
        sem_unlink(semName.c_str());
        semHandle_ = INVALID_SEM_HANDLE;
    }
}

} // namespace SharedMemory
