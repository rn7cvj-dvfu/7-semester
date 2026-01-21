#include <iostream>
#include <thread>
#include <chrono>
#include <process_manager.hpp>

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace ProcessManager;

void printSeparator() {
    std::cout << "\n========================================\n";
}

void testBasicLaunch() {
    printSeparator();
    std::cout << "TEST 1: Базовый запуск процесса\n";
    printSeparator();
    
#ifdef _WIN32
    std::string command = "cmd.exe";
    std::vector<std::string> args = {"/c", "echo", "Hello from child process && timeout /t 2"};
#else
    std::string command = "sh";
    std::vector<std::string> args = {"-c", "echo 'Hello from child process' && sleep 2"};
#endif
    
    std::cout << "Запуск команды: " << command << std::endl;
    
    auto result = ProcessManager::launchProcess(command, args);
    
    if (!result.success) {
        std::cout << "ОШИБКА: " << result.error << std::endl;
        return;
    }
    
    std::cout << "Процесс успешно запущен (handle: " << result.handle << ")" << std::endl;
    std::cout << "Ожидание завершения процесса..." << std::endl;
    
    auto waitResult = ProcessManager::waitForProcess(result.handle);
    
    if (waitResult.success) {
        std::cout << "Процесс завершен с кодом: " << waitResult.exitCode << std::endl;
    } else {
        std::cout << "ОШИБКА при ожидании: " << waitResult.error << std::endl;
    }
    
    ProcessManager::closeHandle(result.handle);
}

void testMultipleProcesses() {
    printSeparator();
    std::cout << "TEST 2: Запуск нескольких процессов одновременно\n";
    printSeparator();
    
    const int processCount = 3;
    ProcessHandle handles[processCount];
    
    for (int i = 0; i < processCount; ++i) {
#ifdef _WIN32
        std::string command = "cmd.exe";
        std::vector<std::string> args = {
            "/c", 
            "echo Process " + std::to_string(i+1) + " && timeout /t " + std::to_string(i+1)
        };
#else
        std::string command = "sh";
        std::vector<std::string> args = {
            "-c", 
            "echo 'Process " + std::to_string(i+1) + "' && sleep " + std::to_string(i+1)
        };
#endif
        
        std::cout << "Запуск процесса " << (i+1) << "..." << std::endl;
        auto result = ProcessManager::launchProcess(command, args);
        
        if (result.success) {
            handles[i] = result.handle;
            std::cout << "  Процесс " << (i+1) << " запущен (handle: " << result.handle << ")" << std::endl;
        } else {
            std::cout << "  ОШИБКА: " << result.error << std::endl;
            handles[i] = 0;
        }
    }
    
    std::cout << "\nВсе процессы запущены. Ожидание завершения..." << std::endl;
    
    for (int i = 0; i < processCount; ++i) {
        if (handles[i] != 0) {
            std::cout << "\nОжидание процесса " << (i+1) << "..." << std::endl;
            auto waitResult = ProcessManager::waitForProcess(handles[i]);
            
            if (waitResult.success) {
                std::cout << "  Процесс " << (i+1) << " завершен с кодом: " << waitResult.exitCode << std::endl;
            } else {
                std::cout << "  ОШИБКА: " << waitResult.error << std::endl;
            }
            
            ProcessManager::closeHandle(handles[i]);
        }
    }
}

void testProcessStatus() {
    printSeparator();
    std::cout << "TEST 3: Проверка статуса процесса\n";
    printSeparator();
    
#ifdef _WIN32
    std::string command = "cmd.exe";
    std::vector<std::string> args = {"/c", "timeout /t 3"};
#else
    std::string command = "sleep";
    std::vector<std::string> args = {"3"};
#endif
    
    std::cout << "Запуск процесса со сном на 3 секунды..." << std::endl;
    
    auto result = ProcessManager::launchProcess(command, args);
    
    if (!result.success) {
        std::cout << "ОШИБКА: " << result.error << std::endl;
        return;
    }
    
    std::cout << "Процесс запущен. Проверка статуса каждую секунду..." << std::endl;
    
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        bool running = ProcessManager::isProcessRunning(result.handle);
        std::cout << "  Секунда " << (i+1) << ": процесс " << (running ? "работает" : "завершен") << std::endl;
        
        if (!running) {
            break;
        }
    }
    
    auto waitResult = ProcessManager::waitForProcess(result.handle);
    if (waitResult.success) {
        std::cout << "Процесс завершен с кодом: " << waitResult.exitCode << std::endl;
    }
    
    ProcessManager::closeHandle(result.handle);
}

void testTimeout() {
    printSeparator();
    std::cout << "TEST 4: Ожидание с таймаутом\n";
    printSeparator();
    
#ifdef _WIN32
    std::string command = "cmd.exe";
    std::vector<std::string> args = {"/c", "timeout /t 5"};
#else
    std::string command = "sleep";
    std::vector<std::string> args = {"5"};
#endif
    
    std::cout << "Запуск процесса на 5 секунд, ожидание с таймаутом 2 секунды..." << std::endl;
    
    auto result = ProcessManager::launchProcess(command, args);
    
    if (!result.success) {
        std::cout << "ОШИБКА: " << result.error << std::endl;
        return;
    }
    
    auto waitResult = ProcessManager::waitForProcess(result.handle, 2000);
    
    if (waitResult.success) {
        std::cout << "Процесс завершен с кодом: " << waitResult.exitCode << std::endl;
    } else {
        std::cout << "Ожидание завершилось по таймауту (ожидаемо)" << std::endl;
        std::cout << "Сообщение: " << waitResult.error << std::endl;
        
        std::cout << "Принудительное завершение процесса..." << std::endl;
        if (ProcessManager::terminateProcess(result.handle)) {
            std::cout << "Процесс успешно завершен" << std::endl;
        }
    }
    
    ProcessManager::closeHandle(result.handle);
}

void testErrorHandling() {
    printSeparator();
    std::cout << "TEST 5: Обработка ошибок\n";
    printSeparator();
    
    std::string command = "nonexistent_command_12345";
    
    std::cout << "Попытка запустить несуществующую команду: " << command << std::endl;
    
    auto result = ProcessManager::launchProcess(command);
    
    if (!result.success) {
        std::cout << "Ошибка успешно обработана: " << result.error << std::endl;
    } else {
        std::cout << "НЕОЖИДАННО: команда запущена" << std::endl;
        ProcessManager::closeHandle(result.handle);
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "===========================================\n";
    std::cout << "  ТЕСТИРОВАНИЕ БИБЛИОТЕКИ PROCESS MANAGER  \n";
    std::cout << "===========================================\n";
    
#ifdef _WIN32
    std::cout << "Платформа: Windows\n";
#else
    std::cout << "Платформа: UNIX/POSIX\n";
#endif
    
    testBasicLaunch();
    testMultipleProcesses();
    testProcessStatus();
    testTimeout();
    testErrorHandling();
    
    printSeparator();
    std::cout << "ВСЕ ТЕСТЫ ЗАВЕРШЕНЫ\n";
    printSeparator();
    
    std::cout << "\nНажмите Enter для выхода...";
    std::cin.get();
    
    return 0;
}
