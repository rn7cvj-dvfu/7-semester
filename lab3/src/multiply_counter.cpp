#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <process_manager.hpp>
#include <shared_memory.hpp>
#include <threads.hpp>
#include <time.hpp>

using namespace ProcessManager;
using namespace SharedMemory;
using namespace Threads;
using namespace Time;

int main(int argc, char* argv[]) {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Проверка обязательных параметров
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <shmName> <logFileName>" << std::endl;
        return 1;
    }
    
    // Парсинг аргументов командной строки
    std::string shmName = argv[1];
    std::string logFileName = argv[2];

    std::ofstream log = std::ofstream(logFileName, std::ios::app);
    
    if (!log.is_open()) {
        return 1;
    }

    int pid = ProcessManager::getProcessID();
    std::string timeStart = Time::GetCurrentTimeString();

    log << "[" << timeStart << "]\t[MULTIPLY] Starting with args: shmName=" << shmName << "\t| PID: " << pid << std::endl;
    log.flush();
        
    SharedMemoryManager sharedMem(shmName);

    if (!sharedMem.isValid()) {
        log << "[" << timeStart << "]\t[MULTIPLY] ERROR: Failed to open shared memory\t| PID: " << pid << std::endl;
        log.flush();
        return 1;
    }
    
    log << "[" << timeStart << "]\t[MULTIPLY] Shared memory opened successfully\t| PID: " << pid << std::endl;
    log.flush();

    log << "[" << timeStart << "]\tMultiply started\t\t\t| PID: " << pid << std::endl;
    log.flush();

    sharedMem.lock();
    int oldValue = sharedMem.getData()->counter;
    sharedMem.getData()->counter *= 2;
    int newValue = sharedMem.getData()->counter;
    sharedMem.unlock();
    
    log << "[" << timeStart << "]\t[MULTIPLY] Counter: " << oldValue << " -> " << newValue << "\t| PID: " << pid << std::endl;
    log.flush();
    
    std::string timeEnd = Time::GetCurrentTimeString();
    log << "[" << timeEnd << "]\tMultiply finished\t\t\t| PID: " << pid << std::endl;
    log.flush();
    log.close();

    return 0;
}
