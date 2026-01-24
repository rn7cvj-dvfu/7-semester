#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <string>
#include <process_manager.hpp>
#include <shared_memory.hpp>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <termios.h>
#endif

using namespace ProcessManager;
using namespace SharedMemory;

// Глобальные переменные
std::atomic<bool> running(true);
std::atomic<bool> copy1Running(false);
std::atomic<bool> copy2Running(false);
ProcessHandle copy1Handle = NULL;
ProcessHandle copy2Handle = NULL;

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
    
#ifdef _WIN32
    localtime_s(&bt, &timer);
#else
    localtime_r(&timer, &bt);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

int64_t getCurrentPid() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

std::string g_logFileName = "process.log";

void writeLog(const std::string& message) {
    std::ofstream logFile(g_logFileName, std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}

// Таймер на 300 мс - увеличение счетчика
void counterTimer(SharedMemoryManager& shm) {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (running) {
            shm.lock();
            shm.getData()->counter++;
            shm.unlock();
        }
    }
}

// Таймер на 1 секунду - запись в лог
void logTimer(SharedMemoryManager& shm, int64_t pid, bool isMaster) {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (running && isMaster) {
            shm.lock();
            int64_t counterValue = shm.getData()->counter;
            shm.unlock();
            
            std::ostringstream oss;
            oss << "[MAIN] Time: " << getCurrentTimestamp() 
                << " | PID: " << pid 
                << " | Counter: " << counterValue;
            writeLog(oss.str());
        }
    }
}

std::string g_incrementExe = "increment_counter";
std::string g_multiplyExe = "multiply_counter";
std::string g_shmName = "lab3_counter";

// Таймер на 3 секунды - запуск копий
void copyLaunchTimer(SharedMemoryManager& shm, int64_t pid, bool isMaster) {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        if (!running || !isMaster) continue;
        
        // Проверяем и запускаем copy1
        if (copy1Running) {
            if (!isProcessRunning(copy1Handle)) {
                copy1Running = false;
                closeHandle(copy1Handle);
            }
        }
        
        if (!copy1Running) {
#ifdef _WIN32
            std::string exePath = g_incrementExe + ".exe";
#else
            std::string exePath = "./" + g_incrementExe;
#endif
            std::vector<std::string> args = {g_shmName, g_logFileName};
            LaunchResult result = launchProcess(exePath, args, true);
            if (result.success) {
                copy1Handle = result.handle;
                copy1Running = true;
                writeLog("[LAUNCHER] Copy1 started successfully");
            } else {
                writeLog("[LAUNCHER] Failed to start copy1: " + result.error);
            }
        } else {
            writeLog("[LAUNCHER] Copy1 still running, skipping launch");
        }
        
        // Проверяем и запускаем copy2
        if (copy2Running) {
            if (!isProcessRunning(copy2Handle)) {
                copy2Running = false;
                closeHandle(copy2Handle);
            }
        }
        
        if (!copy2Running) {
#ifdef _WIN32
            std::string exePath = g_multiplyExe + ".exe";
#else
            std::string exePath = "./" + g_multiplyExe;
#endif
            std::vector<std::string> args = {g_shmName, g_logFileName};
            LaunchResult result = launchProcess(exePath, args, true);
            if (result.success) {
                copy2Handle = result.handle;
                copy2Running = true;
                writeLog("[LAUNCHER] Copy2 started successfully");
            } else {
                writeLog("[LAUNCHER] Failed to start copy2: " + result.error);
            }
        } else {
            writeLog("[LAUNCHER] Copy2 still running, skipping launch");
        }
    }
}

// Поток для обработки ввода пользователя
void inputHandler(SharedMemoryManager& shm) {
    std::cout << "Commands:\n";
    std::cout << "  set <value> - Set counter value\n";
    std::cout << "  exit        - Exit program\n\n";
    
    while (running) {
        std::string command;
        std::cout << "> ";
        std::getline(std::cin, command);
        
        if (command == "exit") {
            running = false;
            break;
        } else if (command.substr(0, 3) == "set") {
            try {
                int64_t newValue = std::stoll(command.substr(4));
                shm.lock();
                shm.getData()->counter = newValue;
                shm.unlock();
                std::cout << "Counter set to " << newValue << std::endl;
            } catch (...) {
                std::cout << "Invalid value. Usage: set <number>" << std::endl;
            }
        } else if (!command.empty()) {
            std::cout << "Unknown command. Use 'set <value>' or 'exit'" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    // Проверка обязательных параметров
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <shmName> <logFileName> <incrementExe> <multiplyExe>" << std::endl;
        std::cerr << "Example: " << argv[0] << " lab3_counter process.log increment_counter multiply_counter" << std::endl;
        return 1;
    }
    
    // Парсинг аргументов командной строки
    std::string shmName = argv[1];
    std::string logFileName = argv[2];
    std::string incrementExe = argv[3];
    std::string multiplyExe = argv[4];
    
    int64_t pid = getCurrentPid();
    std::string startTime = getCurrentTimestamp();
    
    // Подключаемся к разделяемой памяти
    SharedMemoryManager shm(shmName);
    
    if (!shm.isValid()) {
        std::cerr << "Failed to initialize shared memory" << std::endl;
        return 1;
    }
    
    // Записываем информацию о запуске
    std::ostringstream startMsg;
    startMsg << "[MAIN] PID: " << pid << " | Start: " << startTime;
    writeLog(startMsg.str());
    
    // Пытаемся стать мастер-процессом
    bool isMaster = shm.tryBecomeMaster(pid);
    
    if (isMaster) {
        std::cout << "This process is MASTER (PID: " << pid << ")" << std::endl;
        writeLog("[MAIN] This process became MASTER");
    } else {
        std::cout << "This process is SLAVE (PID: " << pid << ")" << std::endl;
        writeLog("[MAIN] This process is SLAVE");
    }
    
    // Устанавливаем глобальное имя лог-файла
    g_logFileName = logFileName;
    
    // Запускаем потоки
    std::thread counterThread(counterTimer, std::ref(shm));
    std::thread logThread(logTimer, std::ref(shm), pid, isMaster);
    std::thread inputThread(inputHandler, std::ref(shm));
    
    std::thread copyThread;
    if (isMaster) {
        copyThread = std::thread(copyLaunchTimer, std::ref(shm), pid, isMaster);
    }
    
    // Ожидаем завершения
    inputThread.join();
    
    running = false;
    
    counterThread.join();
    logThread.join();
    if (isMaster && copyThread.joinable()) {
        copyThread.join();
    }
    
    // Освобождаем роль мастера
    if (isMaster) {
        shm.releaseMaster();
    }
    
    // Завершаем копии если они запущены
    if (copy1Running && copy1Handle != NULL) {
        terminateProcess(copy1Handle);
        closeHandle(copy1Handle);
    }
    if (copy2Running && copy2Handle != NULL) {
        terminateProcess(copy2Handle);
        closeHandle(copy2Handle);
    }
    
    std::string endTime = getCurrentTimestamp();
    std::ostringstream endMsg;
    endMsg << "[MAIN] PID: " << pid << " | End: " << endTime;
    writeLog(endMsg.str());
    
    return 0;
}
