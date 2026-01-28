#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <date_time.hpp>
#include <virtual_com_port.hpp>

#ifdef _WIN32
    #include <windows.h>
#endif


int main(int argc, char* argv[]) {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    if (argc < 6) {
        std::cerr << "Usage: " << argv[0] << " <comPortName> <minValue> <maxValue> <interval (ms)> <randomShift (ms)>" << std::endl;
        return 1;
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    std::string com_port_name = argv[1];

    int min_value = std::stoi(argv[2]);
    int max_value = std::stoi(argv[3]);

    int interval_ms = std::stoi(argv[4]); 
    int random_shift_ms = std::stoi(argv[5]);
    
    VirtualCOM::VirtualComPort com_port(com_port_name);

    if (!com_port.isOpen()) {
        std::cerr << "Не удалось открыть COM порт: " << com_port_name << std::endl;
        return 1;
    }

    std::cout << "Sensor started. Sending data to " << com_port_name << std::endl;
    std::cout << "Value range: [" << min_value << ", " << max_value << "]" << std::endl;
    std::cout << "Interval: " << interval_ms << " ms ± " << random_shift_ms << " ms" << std::endl;

    while (true) {
        int sensor_value = min_value + std::rand() % (max_value - min_value + 1);
        
        int shift = (std::rand() % (2 * random_shift_ms + 1)) - random_shift_ms;
        int wait_time = interval_ms + shift;
        
        std::string now_str = std::to_string(std::time(nullptr));

        std::string message = std::to_string(sensor_value) +"|" + now_str + "\n";
        int written = com_port.write(message);
        
        if (written > 0) {
            std::cout << "Sent: " << message << " (wait: " << wait_time << " ms)" << std::endl;
        } else {
            std::cerr << "Error writing to port!" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    }

    return 0;
}