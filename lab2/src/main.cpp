#include <iostream>
#include <thread>
#include <chrono>
#include <process_manager.hpp>

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace ProcessManager;

#ifdef _WIN32
    std::string terminal = "cmd.exe";
    std::vector<std::string> hello_world_args = { "/C echo Hello, World! && pause" };
#endif

#ifdef __unix__
    std::string terminal = "gnome-terminal";
    std::vector<std::string> hello_world_args = { "--wait", "--", "bash", "-c", "echo 'Hello, World!'; echo 'Нажмите Enter для выхода...';  read" };
#endif

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

    // ProcessManager::LaunchResult launch_result = ProcessManager::launchTerminal(hello_world_args);
    ProcessManager::LaunchResult launch_result = ProcessManager::launchProcess(terminal, hello_world_args);

    if (!launch_result.success) {
        std::cerr << "Ошибка запуска процесса: " << launch_result.error << "\n";
        return 1;
    }

    bool is_running = ProcessManager::isProcessRunning(launch_result.handle);

    std::cout << "Процесс с PID: " << launch_result.handle <<  " " << (is_running ? "запущен" : "не запущен") << "\n";

    WaitResult wait_result = ProcessManager::waitForProcess(launch_result.handle);

    if (!wait_result.success) {
        std::cerr << "Ошибка ожидания процесса: " << wait_result.error << "\n";
        return 1;
    }

#ifdef _WIN32
    std::cout << "\nНажмите Enter для выхода...";
    std::cin.get();
#endif  
    
    return 0;
}
