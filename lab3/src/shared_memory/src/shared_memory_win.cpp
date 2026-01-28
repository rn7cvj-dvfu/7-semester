#include "shared_memory.hpp"
#include <cstring>

namespace SharedMemory {

SharedMemoryManager::SharedMemoryManager(const std::string& name)
    : shm_handle_(INVALID_SHM_HANDLE)
    , sem_handle_(INVALID_SEM_HANDLE)
    , data_(nullptr)
    , name_(name)
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
        WaitForSingleObject(sem_handle_, INFINITE);
    }
}

void SharedMemoryManager::unlock() {
    if (sem_handle_ != INVALID_SEM_HANDLE) {
        ReleaseSemaphore(sem_handle_, 1, NULL);
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
    std::string map_name = "Local\\" + name_;
    shm_handle_ = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(SharedData),
        map_name.c_str()
    );
    
    return shm_handle_ != INVALID_SHM_HANDLE;
}

bool SharedMemoryManager::openSharedMemory() {
    std::string map_name = "Local\\" + name_;
    shm_handle_ = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        map_name.c_str()
    );
    
    return shm_handle_ != INVALID_SHM_HANDLE;
}

bool SharedMemoryManager::mapSharedMemory() {
    if (shm_handle_ == INVALID_SHM_HANDLE) return false;
    
    data_ = static_cast<SharedData*>(
        MapViewOfFile(shm_handle_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData))
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
    if (shm_handle_ != INVALID_SHM_HANDLE) {
        CloseHandle(shm_handle_);
        shm_handle_ = INVALID_SHM_HANDLE;
    }
}

void SharedMemoryManager::destroySharedMemory() {
    unmapSharedMemory();
    closeSharedMemory();
}

bool SharedMemoryManager::createSemaphore() {
    std::string sem_name = name_ + "_sem";
    sem_handle_ = CreateSemaphoreA(
        NULL,
        1,
        1,
        sem_name.c_str()
    );
    
    return sem_handle_ != INVALID_SEM_HANDLE;
}

bool SharedMemoryManager::openSemaphore() {
    std::string sem_name = name_ + "_sem";
    sem_handle_ = OpenSemaphoreA(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        sem_name.c_str()
    );
    
    return sem_handle_ != INVALID_SEM_HANDLE;
}

void SharedMemoryManager::closeSemaphore() {
    if (sem_handle_ != INVALID_SEM_HANDLE) {
        CloseHandle(sem_handle_);
        sem_handle_ = INVALID_SEM_HANDLE;
    }
}

void SharedMemoryManager::destroySemaphore() {
    closeSemaphore();
}


} 
