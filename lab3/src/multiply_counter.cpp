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

std::string g_shmName = "lab3_counter";
std::string g_logFileName = "./logs/log.log";

int main(int argc, char* argv[]) {

        
    SharedMemoryManager sharedMem(g_shmName);

    if (!sharedMem.isValid()) {
        std::cerr << "Error initializing shared memory." << std::endl;
        return 1;
    }
    

    int pid = ProcessManager::getProcessID();
    std::string timeStart = Time::GetCurrentTimeString();

    std::ofstream log = std::ofstream(g_logFileName, std::ios::app);

 
    if (!log.is_open()){
        std::cerr <<"ERROR: Failed to open log file" << std::endl;
        return 1;
    }

    log << "[" << timeStart << "]\tMultiply started\t\t| PID: " << pid << std::endl;
    log.flush();

    sharedMem.lock();
    sharedMem.getData()->counter *= 2;
    sharedMem.unlock();
    
    std::string timeEnd = Time::GetCurrentTimeString();
    log << "[" << timeEnd << "]\tMultiply finished\t\t| PID: " << pid << std::endl;
    log.flush();
    log.close();

    return 0;
}
