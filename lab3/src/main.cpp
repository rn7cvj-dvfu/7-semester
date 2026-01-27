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
            _log = std::ofstream(_logFilePath, std::ios::trunc);
        
            if (!_log.is_open()) {
                std::cerr << "Error opening log file." << std::endl;
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

    void Main() override {
        while (true) {
            IncrementThread::Sleep(0.3);
            _sharedMemory->lock();
            _sharedMemory->getData()->counter++;
            _sharedMemory->unlock();
            CancelPoint();
        }
    }

private:
    SharedMemoryManager *_sharedMemory;
};

class SpawnThread : public Thread { 

public:

    SpawnThread(
        const std::string& executablePath,
        const std::vector<std::string>& args,
        int sleepSeconds = 0
    ) : _executablePath(executablePath), _args(args), _sleepSeconds(sleepSeconds), _processHandle(nullptr) {
    }

    int MainStart() override {
       return 0;
    }

    void Main() override {

        while (true) {
            SpawnThread::Sleep(_sleepSeconds);    
         
            bool running = ProcessManager::isProcessRunning(_processHandle);
            if (running) {
                continue;
            }  
            ProcessManager::closeHandle(_processHandle);
        
            LaunchResult result = ProcessManager::launchProcess(_executablePath, _args);
            if (result.success) {
                _processHandle = result.handle;			// мьютекс должен быть залочен тут!;
            } else {
                std::cerr << "Failed to launch process: " << result.error << std::endl;
            }
                 
            CancelPoint();
        }
    }

    void MainQuit() override {
        ProcessManager::terminateProcess(_processHandle);
        ProcessManager::closeHandle(_processHandle);
    }

private:

    int _sleepSeconds;
    ProcessManager::ProcessHandle _processHandle;

    std::string _executablePath;
    const std::vector<std::string>& _args;

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
        { shmName ,logFileName },
        3
    );


    // SpawnThread multiplySpawnerThread(
    //     multiplyExe,
    //     { shmName, logFileName },
    //     3
    // );

 
    loggerThread.Start();
    incrementThread.Start();
    incrementSpawnerThread.Start();
    // multiplySpawnerThread.Start();
    
    loggerThread.WaitStartup();
    incrementThread.WaitStartup();
    incrementSpawnerThread.WaitStartup();
    // multiplySpawnerThread.WaitStartup();

    std::cout << "set <int> - установить значение счетчика\nexit - завершение программы" << std::endl;
    
    std::string cmd;
    int value;
    
    while(true){

        std::cin >> cmd;


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
        std::cout << "Unknown command." << std::endl;

    }

    loggerThread.Stop();
    incrementThread.Stop();
    incrementSpawnerThread.Stop();
    // multiplySpawnerThread.Stop();   

    loggerThread.Join();   
    incrementThread.Join();
    incrementSpawnerThread.Join();
    // multiplySpawnerThread.Join();
    
#ifdef _WIN32
    
    std::cout << "\nНажмите Enter для выхода...";
    std::cin.get();

#endif      

    return 0;
}
