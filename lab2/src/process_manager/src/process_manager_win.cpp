#include "process_manager.hpp"
#include <windows.h>
#include <sstream>

namespace ProcessManager {

    LaunchResult launchProcess(
        const std::string& command, 
        const std::vector<std::string>& args,
        bool silent
    ) {
        LaunchResult result;
        result.success = false;
        result.handle = NULL;
        
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        std::wstring cmd_line = internal::buildCommandLine(command, args);

        std::vector<wchar_t> cmd_line_buffer(cmd_line.begin(), cmd_line.end());
        cmd_line_buffer.push_back(L'\0');
        
        DWORD creation_flags = silent ? CREATE_NO_WINDOW : CREATE_NEW_CONSOLE;

        if (!CreateProcessW(
                NULL,
                cmd_line_buffer.data(),
                NULL,
                NULL,
                FALSE,
                creation_flags,
                NULL,
                NULL,
                &si,
                &pi
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

    LaunchResult launchTerminal(const std::vector<std::string>& args) {
        LaunchResult result;
        result.success = false;
        result.handle = NULL;
        
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        std::wstring cmd_line = L"cmd.exe";
        
        for (const auto& arg : args) {
            cmd_line += L" " + internal::stringToWString(arg);
        }
        
        std::vector<wchar_t> cmd_line_buffer(cmd_line.begin(), cmd_line.end());
        cmd_line_buffer.push_back(L'\0');
        
        if (!CreateProcessW(
                NULL,
                cmd_line_buffer.data(),
                NULL,
                NULL,
                FALSE,
                CREATE_NEW_CONSOLE,
                NULL,
                NULL,
                &si,
                &pi
            )) {
            DWORD error = GetLastError();
            result.error = "Failed to create terminal process. Error code: " + std::to_string(error);
            return result;
        }
        
        CloseHandle(pi.hThread);
        
        result.success = true;
        result.handle = pi.hProcess;
        
        return result;
    }

    WaitResult waitForProcess(
        ProcessManager::ProcessHandle handle, 
        unsigned int timeout_ms
    ) {
        WaitResult result;
        result.success = false;
        result.exit_code = -1;
        
        if (handle == NULL) {
            result.error = "Invalid process handle";
            return result;
        }
        
        DWORD timeout = (timeout_ms == 0) ? INFINITE : timeout_ms;
        DWORD wait_result = WaitForSingleObject(handle, timeout);
        
        if (wait_result == WAIT_TIMEOUT) {
            result.error = "Wait timeout expired";
            return result;
        }
        
        if (wait_result != WAIT_OBJECT_0) {
            result.error = "Wait failed. Error code: " + std::to_string(GetLastError());
            return result;
        }
        
        DWORD exit_code;
        if (!GetExitCodeProcess(handle, &exit_code)) {
            result.error = "Failed to get exit code. Error code: " + std::to_string(GetLastError());
            return result;
        }
        
        result.success = true;
        result.exit_code = static_cast<int>(exit_code);
        
        return result;
    }

    WaitResult waitForTerminal(ProcessManager::ProcessHandle handle, unsigned int timeout_ms) {
        return waitForProcess(handle, timeout_ms);
    }

    bool isProcessRunning(ProcessManager::ProcessHandle handle) {
        if (handle == NULL) return false;
        
        DWORD exit_code;
        if (!GetExitCodeProcess(handle, &exit_code)) {
            return false;
        }
        
        return exit_code == STILL_ACTIVE;
    }

    bool terminateProcess(ProcessManager::ProcessHandle handle) {
        if (handle == NULL) return false;
        return TerminateProcess(handle, 1) != 0;
    }

    void closeHandle(ProcessManager::ProcessHandle handle) {
        if (handle != NULL) {
            CloseHandle(handle);
        }
    }

    int getProcessID() {
        return static_cast<int>(GetCurrentProcessId());
    }

    LaunchResult launchTerminal(const std::string& command) {
        LaunchResult result;
        result.success = false;
        result.handle = NULL;
        
        std::string full_cmd = std::string("cmd.exe /K ") + command;
        std::vector<std::string> args;
        
        return launchProcess("cmd.exe", { "/K", command });
    }

    namespace internal {

        std::wstring stringToWString(const std::string& str) {
            if (str.empty()) return std::wstring();
            
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
            std::wstring wstr_to(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr_to[0], size_needed);
            return wstr_to;
        }

        std::wstring buildCommandLine(
            const std::string& command, 
            const std::vector<std::string>& args
        ) {
            std::wstring cmd_line = L"\"" + stringToWString(command) + L"\"";
            
            for (const auto& arg : args) {
                cmd_line += L" \"" + stringToWString(arg) + L"\"";
            }
            
            return cmd_line;
        }

    } 

}

