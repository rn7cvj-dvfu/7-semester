#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "shared_memory.hpp"

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
#endif

using namespace SharedMemory;

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

int main(int argc, char* argv[]) {
    // Проверка обязательных параметров
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <shmName> <logFileName>" << std::endl;
        return 1;
    }
    
    // Парсинг аргументов командной строки
    std::string shmName = argv[1];
    std::string logFileName = argv[2];
    
    int64_t pid = getCurrentPid();
    std::string startTime = getCurrentTimestamp();
    
    // Устанавливаем имя лог-файла
    g_logFileName = logFileName;
    
    // Подключаемся к разделяемой памяти
    SharedMemoryManager shm(shmName);
    
    if (!shm.isValid()) {
        std::cerr << "Failed to connect to shared memory" << std::endl;
        return 1;
    }
    
    // Записываем информацию о запуске
    std::ostringstream startMsg;
    startMsg << "[COPY1] PID: " << pid << " | Start: " << startTime;
    writeLog(startMsg.str());
    
    // Увеличиваем счетчик на 10
    shm.lock();
    shm.getData()->counter += 10;
    int64_t newValue = shm.getData()->counter;
    shm.unlock();
    
    std::ostringstream counterMsg;
    counterMsg << "[COPY1] PID: " << pid << " | Counter increased by 10, new value: " << newValue;
    writeLog(counterMsg.str());
    
    // Записываем информацию о завершении
    std::string endTime = getCurrentTimestamp();
    std::ostringstream endMsg;
    endMsg << "[COPY1] PID: " << pid << " | End: " << endTime;
    writeLog(endMsg.str());
    
    return 0;
}
