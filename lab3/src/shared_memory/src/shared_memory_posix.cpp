#include "shared_memory.hpp"
#include <cstring>

namespace SharedMemory {

SharedMemoryManager::SharedMemoryManager(const std::string& name)
    : shm_handle_(INVALID_SHM_HANDLE)
    , sem_handle_(INVALID_SEM_HANDLE)
    , data_(nullptr)
    , name_("/" + name)
    , is_creator_(false)
{
    if (!openSharedMemory()) {
        if (createSharedMemory()) {
            is_creator_ = true;
        }
    }

    if (shm_handle_ != INVALID_SHM_HANDLE) {
        mapSharedMemory();
    }

    std::string sem_name = name_ + "_sem";
    if (!openSemaphore()) {
        createSemaphore();
    }

    if (isValid() && is_creator_) {
        data_->counter = 0;
        data_->process_count = 0;
        data_->is_master_active = false;
        data_->master_pid = 0;
    }

    if (isValid()) {
        lock();
        data_->process_count++;
        unlock();
    }
}

SharedMemoryManager::~SharedMemoryManager() {
    if (isValid()) {
        lock();
        data_->process_count--;
        int count = data_->process_count;
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
    return shm_handle_ != INVALID_SHM_HANDLE && 
           sem_handle_ != INVALID_SEM_HANDLE && 
           data_ != nullptr;
}

void SharedMemoryManager::lock() {
    if (sem_handle_ != INVALID_SEM_HANDLE) {
        sem_wait(sem_handle_);
    }
}

void SharedMemoryManager::unlock() {
    if (sem_handle_ != INVALID_SEM_HANDLE) {
        sem_post(sem_handle_);
    }
}

SharedData* SharedMemoryManager::getData() {
    return data_;
}

bool SharedMemoryManager::tryBecomeMaster(int64_t pid) {
    if (!isValid()) return false;
    
    lock();
    bool result = false;
    if (!data_->is_master_active) {
        data_->is_master_active = true;
        data_->master_pid = pid;
        result = true;
    }
    unlock();
    
    return result;
}

void SharedMemoryManager::releaseMaster() {
    if (!isValid()) return;
    
    lock();
    data_->is_master_active = false;
    data_->master_pid = 0;
    unlock();
}

bool SharedMemoryManager::isMaster(int64_t pid) const {
    if (!isValid()) return false;
    return data_->is_master_active && data_->master_pid == pid;
}

bool SharedMemoryManager::createSharedMemory() {
    shm_handle_ = shm_open(name_.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
    
    if (shm_handle_ != INVALID_SHM_HANDLE) {
        ftruncate(shm_handle_, sizeof(SharedData));
        return true;
    }
    
    return false;
}

bool SharedMemoryManager::openSharedMemory() {
    shm_handle_ = shm_open(name_.c_str(), O_RDWR, 0644);
    return shm_handle_ != INVALID_SHM_HANDLE;
}

bool SharedMemoryManager::mapSharedMemory() {
    if (shm_handle_ == INVALID_SHM_HANDLE) return false;
    
    void* addr = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, 
                      MAP_SHARED, shm_handle_, 0);
    
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
    if (shm_handle_ != INVALID_SHM_HANDLE) {
        close(shm_handle_);
        shm_handle_ = INVALID_SHM_HANDLE;
    }
}

void SharedMemoryManager::destroySharedMemory() {
    unmapSharedMemory();
    
    if (shm_handle_ != INVALID_SHM_HANDLE) {
        close(shm_handle_);
        shm_unlink(name_.c_str());
        shm_handle_ = INVALID_SHM_HANDLE;
    }
}

bool SharedMemoryManager::createSemaphore() {
    std::string sem_name = name_ + "_sem";
    sem_handle_ = sem_open(sem_name.c_str(), O_CREAT | O_EXCL, 0644, 1);
    
    return sem_handle_ != INVALID_SEM_HANDLE;
}

bool SharedMemoryManager::openSemaphore() {
    std::string sem_name = name_ + "_sem";
    sem_handle_ = sem_open(sem_name.c_str(), 0);
    
    return sem_handle_ != INVALID_SEM_HANDLE;
}

void SharedMemoryManager::closeSemaphore() {
    if (sem_handle_ != INVALID_SEM_HANDLE) {
        sem_close(sem_handle_);
        sem_handle_ = INVALID_SEM_HANDLE;
    }
}

void SharedMemoryManager::destroySemaphore() {
    if (sem_handle_ != INVALID_SEM_HANDLE) {
        std::string sem_name = name_ + "_sem";
        sem_close(sem_handle_);
        sem_unlink(sem_name.c_str());
        sem_handle_ = INVALID_SEM_HANDLE;
    }
}


