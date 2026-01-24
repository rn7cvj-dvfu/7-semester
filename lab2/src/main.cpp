#include <iostream>
#include <thread>
#include <chrono>
#include <process_manager.hpp>

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace ProcessManager;


#pragma region TERMIAL_COMMANDS

#ifdef _WIN32
 
    std::string terminal = "cmd.exe";
    std::vector<std::string> helloWorldArgs = { "/C", "echo Hello, World! && pause" };

#endif

#ifdef __unix__
 
    std::string terminal = "gnome-terminal";
    std::vector<std::string> helloWorldArgs = { "--wait", "--", "bash", "-c", "echo 'Hello, World!'; echo 'Нажмите Enter для выхода...';  read" };
#endif

#pragma endregion


int main() {
    
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
#ifdef _WIN32
    std::cout << "Платформа: Windows\n";
#elif defined(__unix__)
    std::cout << "Платформа: UNIX/POSIX\n";
#else
    std::cout << "Платформа: Неизвестная\n";
    return 1;
#endif


    // ProcessManager::LaunchResult launchResult =  ProcessManager::launchTerminal("echo 'Hello, World!'; bash");
    LaunchResult launchResult =  ProcessManager::launchProcess(terminal, helloWorldArgs);

    if (!launchResult.success) {
        std::cerr << "Ошибка запуска процесса: " << launchResult.error << "\n";
        return 1;
    }

    bool isRunning = ProcessManager::isProcessRunning(launchResult.handle);

    std::cout << "Процесс с PID: " << launchResult.handle <<  " " << (isRunning ? "запущен" : "не запущен") << "\n";


    WaitResult waitResult = ProcessManager::waitForProcess(launchResult.handle);

    if (!waitResult.success) {
        std::cerr << "Ошибка ожидания процесса: " << waitResult.error << "\n";
        return 1;
    }


#ifdef _WIN32
    
    std::cout << "\nНажмите Enter для выхода...";
    std::cin.get();

#endif  
    
    return 0;
}
