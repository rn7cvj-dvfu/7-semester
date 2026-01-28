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

std::string g_shm_name = "lab3_counter";
std::string g_log_file_name = "./logs/log.log";

int main(int argc, char* argv[]) {
        
    SharedMemoryManager shared_mem(g_shm_name);

    if (!shared_mem.isValid()) {
        std::cerr << "Error initializing shared memory." << std::endl;
        return 1;
    }

    int pid = ProcessManager::getProcessID();
    std::string time_start = Time::getCurrentTimeString();

    std::ofstream log = std::ofstream(g_log_file_name, std::ios::app);

    if (!log.is_open()){
        std::cerr <<"ERROR: Failed to open log file" << std::endl;
        return 1;
    }

    log << "[" << time_start << "]\tMultiply started\t\t| PID: " << pid << std::endl;
    log.flush();

    shared_mem.lock();
    shared_mem.getData()->counter *= 2;
    shared_mem.unlock();
    
    std::string time_end = Time::getCurrentTimeString();
    log << "[" << time_end << "]\tMultiply finished\t\t| PID: " << pid << std::endl;
    log.flush();
    log.close();

    return 0;
}
