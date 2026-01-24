#include "process_manager.hpp"
#include <windows.h>
#include <sstream>


namespace ProcessManager {

namespace internal {

std::wstring stringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstrTo[0], size_needed);
    return wstrTo;
}

std::wstring buildCommandLine(
    const std::string& command, 
    const std::vector<std::string>& args
) {
    std::wstring cmdLine = L"\"" + stringToWString(command) + L"\"";
    
    for (const auto& arg : args) {
        cmdLine += L" \"" + stringToWString(arg) + L"\"";
    }
    
    return cmdLine;
}

} 

LaunchResult launchProcess(
    const std::string& command, 
    const std::vector<std::string>& args
) {
    LaunchResult result;
    result.success = false;
    result.handle = NULL;
    
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    std::wstring cmdLine = internal::buildCommandLine(command, args);

    std::vector<wchar_t> cmdLineBuffer(cmdLine.begin(), cmdLine.end());
    cmdLineBuffer.push_back(L'\0');
    
    if (!CreateProcessW(
            NULL,                   // Имя модуля (NULL = использовать командную строку)
            cmdLineBuffer.data(),   // Командная строка
            NULL,                   // Атрибуты безопасности процесса
            NULL,                   // Атрибуты безопасности потока
            FALSE,                  // Наследование дескрипторов
            CREATE_NEW_CONSOLE,     // Флаги создания (новая консоль для фонового процесса)
            NULL,                   // Окружение
            NULL,                   // Текущая директория
            &si,                    // Информация о запуске
            &pi                     // Информация о процессе
        )) {
        DWORD error = GetLastError();
        result.error = "Failed to create process. Error code: " + std::to_string(error);
        return result;
    }
    
    CloseHandle(pi.hThread);
    
    result.success = true;
    result.handle = pi.hProcess;
    
    return result;
}

WaitResult waitForProcess(
    ProcessHandle handle, 
    unsigned int timeoutMs
) {
    WaitResult result;
    result.success = false;
    result.exitCode = -1;
    
    if (handle == NULL) {
        result.error = "Invalid process handle";
        return result;
    }
    
    DWORD timeout = (timeoutMs == 0) ? INFINITE : timeoutMs;
    DWORD waitResult = WaitForSingleObject(handle, timeout);
    
    if (waitResult == WAIT_TIMEOUT) {
        result.error = "Wait timeout expired";
        return result;
    }
    
    if (waitResult != WAIT_OBJECT_0) {
        result.error = "Wait failed. Error code: " + std::to_string(GetLastError());
        return result;
    }
    
    DWORD exitCode;
    if (!GetExitCodeProcess(handle, &exitCode)) {
        result.error = "Failed to get exit code. Error code: " + std::to_string(GetLastError());
        return result;
    }
    
    result.success = true;
    result.exitCode = static_cast<int>(exitCode);
    
    return result;
}

bool isProcessRunning(ProcessHandle handle) {
    if (handle == NULL) return false;
    
    DWORD exitCode;
    if (!GetExitCodeProcess(handle, &exitCode)) {
        return false;
    }
    
    return exitCode == STILL_ACTIVE;
}

bool terminateProcess(ProcessHandle handle) {
    if (handle == NULL) return false;
    return TerminateProcess(handle, 1) != 0;
}

void closeHandle(ProcessHandle handle) {
    if (handle != NULL) {
        CloseHandle(handle);
    }
}

LaunchResult launchTerminal(const std::string& command) {
    LaunchResult result;
    result.success = false;
    result.handle = NULL;
    
    // На Windows используем cmd.exe
    std::string fullCmd = std::string("cmd.exe /K ") + command;
    std::vector<std::string> args;
    
    return launchProcess("cmd.exe", { "/K", command });
}

WaitResult waitForTerminal(ProcessHandle handle, unsigned int timeoutMs) {
    return waitForProcess(handle, timeoutMs);
}

}

