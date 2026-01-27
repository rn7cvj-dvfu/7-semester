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

using namespace ProcessManager;
using namespace SharedMemory;


int main(int argc, char* argv[]) {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    // Проверка обязательных параметров
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <shmName> <logFileName> <incrementExe> <multiplyExe>" << std::endl;
        return 1;
    }
    
    
    
    return 0;
}
