#include "process_manager.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
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
    
    // Создаем pipe для обмена информацией об ошибках между родителем и дочерним процессом
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        result.error = "Failed to create pipe: " + std::string(std::strerror(errno));
        return result;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        result.error = "Failed to fork process: " + std::string(std::strerror(errno));
        close(pipefd[0]);
        close(pipefd[1]);
        return result;
    }
    
    if (pid == 0) {
        // Дочерний процесс
        close(pipefd[0]); // Закрываем конец для чтения в дочернем процессе
        
        // Устанавливаем флаг close-on-exec для pipe
        fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);
        
        char** argv = internal::buildArgv(command, args);
        
        // execvp заменяет процесс, поэтому он никогда не вернет управление при успехе
        execvp(command.c_str(), argv);
        
        // Если мы здесь - произошла ошибка в execvp
        int err = errno;
        // Пишем код ошибки в pipe, чтобы родитель мог его прочитать
        ssize_t writeResult = write(pipefd[1], &err, sizeof(err));
        (void)writeResult; // Избегаем неиспользованного предупреждения
        
        internal::freeArgv(argv);
        _exit(1);
    }
    
    // Родительский процесс
    close(pipefd[1]); // Закрываем конец для записи в родительском процессе
    
    // Пытаемся прочитать ошибку из pipe
    int err;
    ssize_t readBytes = read(pipefd[0], &err, sizeof(err));
    close(pipefd[0]);
    
    if (readBytes == sizeof(err)) {
        // Мы получили ошибку из дочернего процесса
        result.error = "Failed to execute command: " + std::string(std::strerror(err));
        
        // Ждем завершения дочернего процесса, чтобы избежать зомби
        int status;
        waitpid(pid, &status, 0);
        
        return result;
    }
    
    if (readBytes == -1) {
        result.error = "Failed to read from pipe: " + std::string(std::strerror(errno));
        
        int status;
        waitpid(pid, &status, 0);
        
        return result;
    }
    
    // readBytes == 0 означает, что pipe был закрыт без записи
    // Это нормально - это значит, что execvp успешно выполнена
    
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

LaunchResult launchTerminal(const std::string& command) {
    LaunchResult result;
    result.success = false;
    result.handle = -1;
    
    // Ищем доступный терминал
    const char* terminals[] = {
        "x-terminal-emulator",
        "gnome-terminal",
        "konsole",
        "xfce4-terminal",
        "xterm",
        "mate-terminal",
        nullptr
    };
    
    std::string terminalCmd;
    for (int i = 0; terminals[i] != nullptr; ++i) {
        std::string checkCmd = std::string("which ") + terminals[i] + " > /dev/null 2>&1";
        if (system(checkCmd.c_str()) == 0) {
            terminalCmd = terminals[i];
            break;
        }
    }
    
    if (terminalCmd.empty()) {
        result.error = "No terminal emulator found";
        return result;
    }
    
    // Формируем аргументы для терминала
    std::vector<std::string> args;
    
    if (terminalCmd == "xterm") {
        args = { "-e", "bash", "-c", command };
    } else if (terminalCmd == "gnome-terminal") {
        args = { "--", "bash", "-c", command };
    } else if (terminalCmd == "konsole") {
        args = { "-e", "bash", "-c", command };
    } else if (terminalCmd == "xfce4-terminal") {
        args = { "-e", "bash", "-c", command };
    } else if (terminalCmd == "mate-terminal") {
        args = { "-e", "bash", "-c", command };
    } else {
        args = { "-e", "bash", "-c", command };
    }
    
    return launchProcess(terminalCmd, args);
}

WaitResult waitForTerminal(ProcessHandle handle, unsigned int timeoutMs) {
    return waitForProcess(handle, timeoutMs);
}

}

