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
    // Проверка обязательных параметров
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <shmName> <logFileName>" << std::endl;
        return 1;
    }
    
    // Парсинг аргументов командной строки
    std::string shmName = argv[1];
    std::string logFileName = argv[2];

        
    SharedMemoryManager sharedMem(shmName);

    if (!sharedMem.isValid()) {
        std::cerr << "Error initializing shared memory." << std::endl;
        return 1;
    }
    

    int pid = ProcessManager::getProcessID();
    std::string time = Time::GetCurrentTimeString();

    std::ofstream log = std::ofstream(logFileName, std::ios::trunc);

    log << "[" << time << "]\tIncrement started\t\t\t| PID: " << pid << std::endl;
    

    sharedMem.lock();
    sharedMem.getData()->counter += 10;
    sharedMem.unlock();
    
    log << "[" << time << "]\tIncrement finished\t\t\t| PID: " << pid << std::endl;

    return 0;
}
