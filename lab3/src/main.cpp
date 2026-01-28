#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <string>
#include <filesystem>

#include <process_manager.hpp>
#include <shared_memory.hpp>
#include <threads.hpp>
#include <time.hpp>

using namespace ProcessManager;
using namespace SharedMemory;
using namespace Threads;
using namespace Time;

class LoggerThread : public Thread{

    public:

        LoggerThread(
            SharedMemoryManager *shared_memory,
            const std::string &log_file_path
        ) : _shared_mem(shared_memory), _log_file_path(log_file_path) {
        }

        int MainStart() override {
        _log = std::ofstream(_log_file_path, std::ios::app);
    
        if (!_log.is_open()) {
            std::cerr << "[LOGGER] ERROR: Failed to open log file" << std::endl;
            return 1;
        }

        int pid = ProcessManager::getProcessID();
        std::string time = Time::getCurrentTimeString();

        _log << "[" << time << "]\tLogger started\t\t\t| PID: " << pid << std::endl;
        _log.flush();
            return 0;
        }

        void Main() override {

            while(true) {
                LoggerThread::Sleep(1.0);
                _shared_mem->lock();
                int value = _shared_mem->getData()->counter;

                int pid = ProcessManager::getProcessID();
                std::string time = Time::getCurrentTimeString();

                _shared_mem->unlock();
              
                _log << "[" << time << "]\tCounter=" << value << "\t\t\t| PID: " << pid << std::endl;
                _log.flush();
                CancelPoint();
            }
        }

        void MainQuit()override {
        int pid = ProcessManager::getProcessID();
        std::string time = Time::getCurrentTimeString();

        _log << "[" << time << "]\tLogger stopping\t\t\t| PID: " << pid << std::endl;
        _log.flush();
        }

private:

    SharedMemory::SharedMemoryManager *_shared_mem;
    std::string _log_file_path;
    std::ofstream _log;
};

class IncrementThread : public Thread {
public:
    IncrementThread(SharedMemoryManager *shared_memory) : _shared_memory(shared_memory) {
    }

    int MainStart() override {
        return 0;
    }

    void Main() override {
        while (true) {
            IncrementThread::Sleep(0.3);
            _shared_memory->lock();
            _shared_memory->getData()->counter++;
            _shared_memory->unlock();
            CancelPoint();
        }
    }

    void MainQuit() override {
    }

private:
    SharedMemoryManager *_shared_memory;
};

class SpawnThread : public Thread { 

public:

    SpawnThread(
        const std::string& executable_path,
        const std::vector<std::string>& args,
        double sleep_seconds = 0.0
    ) : _executable_path(executable_path), _args(args), _sleep_seconds(sleep_seconds), _process_handle(nullptr) {
    }

    int MainStart() override {
        return 0;
    }

    void Main() override {
        while (true) {
            SpawnThread::Sleep(_sleep_seconds);

            if (_process_handle != nullptr) {
                bool running = ProcessManager::isProcessRunning(_process_handle);
                if (running) {
                    continue;
                }  
                ProcessManager::closeHandle(_process_handle);
                _process_handle = nullptr;
            }
        
            LaunchResult result = ProcessManager::launchProcess(_executable_path, {} , true);

            if (result.success) {
                _process_handle = result.handle;
   
                SpawnThread::Sleep(_sleep_seconds);
            } else {
                std::cerr << "[SPAWN:" << _executable_path << "] ERROR: Failed to launch process: " << result.error << std::endl;
             
            }
                 
            CancelPoint();
        }
    }

    void MainQuit() override {
        ProcessManager::terminateProcess(_process_handle);
        ProcessManager::closeHandle(_process_handle);
    }

private:

    double _sleep_seconds;
    ProcessManager::ProcessHandle _process_handle;

    std::string _executable_path;
    std::vector<std::string> _args;

};

int main(int argc, char* argv[]) {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <shmName> <logFileName> <incrementExe> <multiplyExe>" << std::endl;
        return 1;
    }

    std::string shm_name = argv[1];
    std::string log_file_name = argv[2];
    std::string increment_exe = argv[3];
    std::string multiply_exe = argv[4];

    std::string abs_log_path = std::filesystem::absolute(log_file_name).string();

    std::filesystem::create_directories(std::filesystem::path(log_file_name).parent_path());

    SharedMemoryManager shared_mem(shm_name);

    if (!shared_mem.isValid()) {
        std::cerr << "Error initializing shared memory." << std::endl;
        return 1;
    }

    bool is_master = shared_mem.tryBecomeMaster(ProcessManager::getProcessID());
    
    LoggerThread logger_thread(
        &shared_mem,
        log_file_name
    );
    
    IncrementThread increment_thread(&shared_mem);

    SpawnThread increment_spawner_thread(
        increment_exe,
        {  shm_name , abs_log_path    },
        3.0
    );

    SpawnThread multiply_spawner_thread(
        multiply_exe,
        { shm_name, abs_log_path },
        3.0
    );
 
    logger_thread.Start();
    increment_thread.Start();

    if (is_master){
        increment_spawner_thread.Start();
        multiply_spawner_thread.Start();    
    }
    
    logger_thread.WaitStartup();
    increment_thread.WaitStartup();

    if (is_master){
        increment_spawner_thread.WaitStartup();
        multiply_spawner_thread.WaitStartup();
    }
    
    std::string cmd;
    int value;
    
    while(true){
        std::cout << "> " << std::flush;
        std::cin >> cmd;
        
        if (std::cin.eof()) {
            break;
        }
        
        if (std::cin.fail()) {
            break;
        }

        if (cmd == "set"){
            std::cin >> value;
            shared_mem.lock();
            shared_mem.getData()->counter = value;
            shared_mem.unlock();
            continue;
        }
        if (cmd == "exit"){
            break;
        }

    }

    logger_thread.Stop();
    increment_thread.Stop();

    if (is_master){
        increment_spawner_thread.Stop();
        multiply_spawner_thread.Stop();   
    }

    logger_thread.Join();   
    increment_thread.Join();

    if (is_master){
        increment_spawner_thread.Join();
        multiply_spawner_thread.Join();
    }
    
#ifdef _WIN32
    std::cin.get();
#endif      

    return 0;
        
}
