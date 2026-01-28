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
            SharedMemoryManager *sharedMemory,
            const std::string &logFilePath
        ) : _sharedMem(sharedMemory), _logFilePath(logFilePath) {
        }

        int MainStart() override {
        _log = std::ofstream(_logFilePath, std::ios::app);
    
        if (!_log.is_open()) {
            std::cerr << "[LOGGER] ERROR: Failed to open log file" << std::endl;
            return 1;
        }

        int pid = ProcessManager::getProcessID();
        std::string time = Time::GetCurrentTimeString();

        _log << "[" << time << "]\tLogger started\t\t\t| PID: " << pid << std::endl;
        _log.flush();
            return 0;
        }

        void Main() override {
            while(true) {
                LoggerThread::Sleep(1.0);
                _sharedMem->lock();
                int value = _sharedMem->getData()->counter;

                int pid = ProcessManager::getProcessID();
                std::string time = Time::GetCurrentTimeString();

                _sharedMem->unlock();
              
                _log << "[" << time << "]\tCounter=" << value << "\t\t\t| PID: " << pid << std::endl;
                _log.flush();
                CancelPoint();
            }
        }

        void MainQuit()override {
        int pid = ProcessManager::getProcessID();
        std::string time = Time::GetCurrentTimeString();

        _log << "[" << time << "]\tLogger stopping\t\t\t| PID: " << pid << std::endl;
        _log.flush();
        }

private:

    SharedMemory::SharedMemoryManager *_sharedMem;
    std::string _logFilePath;
    std::ofstream _log;
};

class IncrementThread : public Thread {
public:
    IncrementThread(SharedMemoryManager *sharedMemory) : _sharedMemory(sharedMemory) {
    }

    int MainStart() override {
        return 0;
    }

    void Main() override {
        while (true) {
            IncrementThread::Sleep(0.3);
            _sharedMemory->lock();
            _sharedMemory->getData()->counter++;
            _sharedMemory->unlock();
            CancelPoint();
        }
    }

    void MainQuit() override {
    }

private:
    SharedMemoryManager *_sharedMemory;
};

class SpawnThread : public Thread { 

public:

    SpawnThread(
        const std::string& executablePath,
        const std::vector<std::string>& args,
        double sleepSeconds = 0.0
    ) : _executablePath(executablePath), _args(args), _sleepSeconds(sleepSeconds), _processHandle(nullptr) {
    }

    int MainStart() override {
        return 0;
    }

    void Main() override {
        while (true) {
            // // Проверяем процесс только если он был запущен
            // if (_processHandle != nullptr) {
            //     bool running = ProcessManager::isProcessRunning(_processHandle);
            //     if (running) {
            //         SpawnThread::Sleep(0.1); // Короткая проверка
            //         CancelPoint();
            //         continue;
            //     }  
            //     ProcessManager::closeHandle(_processHandle);
            //     _processHandle = nullptr;
            // }
        
            // LaunchResult result = ProcessManager::launchProcess(_executablePath, _args);
            // if (result.success) {
            //     _processHandle = result.handle;
                
   
            //     SpawnThread::Sleep(_sleepSeconds);
            // } else {
            //     std::cerr << "[SPAWN:" << _executablePath << "] ERROR: Failed to launch process: " << result.error << std::endl;
            //     SpawnThread::Sleep(_sleepSeconds);
            // }
                 
            CancelPoint();
        }
    }

    void MainQuit() override {
        ProcessManager::terminateProcess(_processHandle);
        ProcessManager::closeHandle(_processHandle);
    }

private:

    double _sleepSeconds;
    ProcessManager::ProcessHandle _processHandle;

    std::string _executablePath;
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

    std::string shmName = argv[1];
    std::string logFileName = argv[2];
    std::string incrementExe = argv[3];
    std::string multiplyExe = argv[4];

    // // Преобразование в абсолютные пути
    // auto absLogPath = std::filesystem::absolute(logFileName).string();
    // auto absIncrementExe = std::filesystem::absolute(incrementExe).string();
    // auto absMultiplyExe = std::filesystem::absolute(multiplyExe).string();

    std::filesystem::create_directories(std::filesystem::path(logFileName).parent_path());
    
    SharedMemoryManager sharedMem(shmName);


    if (!sharedMem.isValid()) {
        std::cerr << "Error initializing shared memory." << std::endl;
        return 1;
    }
    
    LoggerThread loggerThread(
        &sharedMem,
        logFileName
    );
    
    IncrementThread incrementThread(&sharedMem);

    SpawnThread incrementSpawnerThread(
        incrementExe,
        { shmName, logFileName },
        3.0
    );


    SpawnThread multiplySpawnerThread(
        multiplyExe,
        { shmName, logFileName },
        3.0
    );

 
    loggerThread.Start();
    incrementThread.Start();
    incrementSpawnerThread.Start();
    multiplySpawnerThread.Start();
    
    loggerThread.WaitStartup();
    incrementThread.WaitStartup();
    incrementSpawnerThread.WaitStartup();
    multiplySpawnerThread.WaitStartup();
    
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
            sharedMem.lock();
            sharedMem.getData()->counter = value;
            sharedMem.unlock();
            continue;
        }
        if (cmd == "exit"){
            break;
        }

    }

    loggerThread.Stop();
    incrementThread.Stop();
    incrementSpawnerThread.Stop();
    multiplySpawnerThread.Stop();   

    loggerThread.Join();   
    incrementThread.Join();
    incrementSpawnerThread.Join();
    multiplySpawnerThread.Join();
    
#ifdef _WIN32
    std::cin.get();
#endif      

    return 0;
        
}
