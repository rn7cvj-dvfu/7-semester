#include "process_manager.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <cerrno>
#include <iostream>

namespace ProcessManager {

namespace internal {

char** buildArgv(
    const std::string& command, 
    const std::vector<std::string>& args
) {
    size_t argc = 1 + args.size() + 1;
    char** argv = new char*[argc];
    
    argv[0] = new char[command.length() + 1];
    std::strcpy(argv[0], command.c_str());
    
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i + 1] = new char[args[i].length() + 1];
        std::strcpy(argv[i + 1], args[i].c_str());
    }
    
    argv[argc - 1] = NULL;
    
    return argv;
}

void freeArgv(char** argv) {
    if (argv == NULL) return;
    
    for (int i = 0; argv[i] != NULL; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
}

} // namespace internal

LaunchResult launchProcess(
    const std::string& command, 
    const std::vector<std::string>& args
) {
    LaunchResult result;
    result.success = false;
    result.handle = -1;
    
    pid_t pid = fork();
    
    if (pid < 0) {
        result.error = "Failed to fork process: " + std::string(std::strerror(errno));
        return result;
    }
    
    if (pid == 0) {
        char** argv = internal::buildArgv(command, args);
        
        execvp(command.c_str(), argv);
        
        std::cerr << "Failed to execute " << command << ": " << std::strerror(errno) << std::endl;
        internal::freeArgv(argv);
        _exit(1);
    }
    
    result.success = true;
    result.handle = pid;
    
    return result;
}

WaitResult waitForProcess(
    ProcessHandle handle, 
    unsigned int timeoutMs
) {
    WaitResult result;
    result.success = false;
    result.exitCode = -1;
    
    if (handle <= 0) {
        result.error = "Invalid process handle";
        return result;
    }
    
    int status;
    pid_t waitResult;
    
    if (timeoutMs == 0) {
        waitResult = waitpid(handle, &status, 0);
    } else {
        // Ожидание с таймаутом (опрос каждые 100мс)
        unsigned int elapsed = 0;
        const unsigned int pollInterval = 100;
        
        while (elapsed < timeoutMs) {
            waitResult = waitpid(handle, &status, WNOHANG);
            
            if (waitResult > 0) break;
            if (waitResult < 0) break;
            
            usleep(pollInterval * 1000);
            elapsed += pollInterval;
        }
        
        if (waitResult == 0) {
            result.error = "Wait timeout expired";
            return result;
        }
    }
    
    if (waitResult < 0) {
        result.error = "waitpid failed: " + std::string(std::strerror(errno));
        return result;
    }
    
    if (WIFEXITED(status)) {
        result.success = true;
        result.exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result.success = true;
        result.exitCode = -WTERMSIG(status); // Отрицательный код для сигналов
        result.error = "Process terminated by signal " + std::to_string(WTERMSIG(status));
    } else {
        result.error = "Process did not exit normally";
    }
    
    return result;
}

bool isProcessRunning(ProcessHandle handle) {
    if (handle <= 0) return false;
    
    int status;
    pid_t result = waitpid(handle, &status, WNOHANG);
    
    if (result == 0) {
        return true; // Процесс еще работает
    }
    
    return false;
}

bool terminateProcess(ProcessHandle handle) {
    if (handle <= 0) return false;
    
    if (kill(handle, SIGTERM) == 0) {
        return true;
    }
    
    return false;
}

void closeHandle(ProcessHandle handle) {
    // В POSIX нет необходимости явно закрывать PID
    // Ресурсы освобождаются после waitpid
    (void)handle;
}

}

