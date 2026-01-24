#include "process_manager.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <iostream>

namespace ProcessManager {

    LaunchResult launchProcess(
        const std::string& command, 
        const std::vector<std::string>& args
    ) {
        LaunchResult result;
        result.success = false;
        result.handle = -1;

        ProcessHandle pid = fork();

        if (pid < 0) {
            result.error = "Fork failed: " + std::string(std::strerror(errno));
            return result;
        }

        if (pid == 0){

            char** argv = internal::buildArgv(command, args);

            execvp(command.c_str(), argv);
            std::cerr << "Exec failed: " << std::strerror(errno) << std::endl;
            
            internal::freeArgv(argv);
            
            _exit(1);
        }

        result.success = true;
        result.handle = pid;

        return result;
    }

    LaunchResult launchTerminal(const std::vector<std::string>& args = {}) {
        LaunchResult result;
        result.success = false;
        result.handle = -1;
        
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
        
        std::vector<std::string> processArgs;
        
        if (terminalCmd == "xterm") {
            processArgs = { "-e", "bash", "-c" };
        } else if (terminalCmd == "gnome-terminal") {
            processArgs = { "--wait", "--", "bash", "-c" };
        } else if (terminalCmd == "konsole") {
            processArgs = { "-e", "bash", "-c" };
        } else if (terminalCmd == "xfce4-terminal") {
            processArgs = { "-e", "bash", "-c" };
        } else if (terminalCmd == "mate-terminal") {
            processArgs = { "-e", "bash", "-c" };
        } else {
            processArgs = { "-e", "bash", "-c" };
        }
        
         
        processArgs.insert(processArgs.end(), args.begin(), args.end());    
        
        return launchProcess(terminalCmd, processArgs);
    }

    WaitResult waitForProcess(
        ProcessHandle handle, 
        unsigned int timeoutMs
    ) {
        WaitResult result;
        result.success = false;
        result.exitCode = -1;

        int status;
        pid_t waitResult = waitpid(handle, &status, 0);
        
        if (waitResult == -1) {
            result.error = "waitpid failed: " + std::string(std::strerror(errno));
            return result;
        }

        if (WIFEXITED(status)) {
            result.exitCode = WEXITSTATUS(status);
            result.success = true;
        } else {
            result.error = "Process did not terminate normally";
        }
        
        return result;
    }

   WaitResult waitForTerminal(ProcessHandle handle, unsigned int timeoutMs) {
        return waitForProcess(handle, timeoutMs);
    }

    bool isProcessRunning(ProcessHandle handle) {
        if (handle <= 0) return false;
        

        int result = kill(handle, 0);
        
        if (result == 0) {
            return true;
        }
        
        if (errno == ESRCH) {
            return false;
        }
        
        return true;
    }

    bool terminateProcess(ProcessHandle handle) {
        if (handle <= 0) return false;
        
        if (kill(handle, SIGTERM) == 0) {
            return true;
        }
        
        return false;
    }

    void closeHandle(ProcessHandle handle) {
    
        (void)handle;
    }

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

    }

}

