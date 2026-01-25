#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <date_time.hpp>
#include <virtual_com_port.hpp>

int main(int argc, char* argv[]) {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    if (argc < 6) {
        std::cerr << "Usage: " << argv[0] << " <comPortName> <minValue> <maxValue> <interval (ms)> <randomShift (ms)>" << std::endl;
        return 1;

    }

    // Инициализация генератора случайных чисел
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    std::string comPortName = argv[1];

    int minValue = std::stoi(argv[2]);
    int maxValue = std::stoi(argv[3]);

    int intervalMS = std::stoi(argv[4]); 
    int randomShiftMS = std::stoi(argv[5]);
    
    
    VirtualCOM::VirtualComPort comPort(comPortName);

    if (!comPort.isOpen()) {
        std::cerr << "Не удалось открыть COM порт: " << comPortName << std::endl;
        return 1;
    }



    std::cout << "Sensor started. Sending data to " << comPortName << std::endl;
    std::cout << "Value range: [" << minValue << ", " << maxValue << "]" << std::endl;
    std::cout << "Interval: " << intervalMS << " ms ± " << randomShiftMS << " ms" << std::endl;

    while (true) {

        int sensorValue = minValue + std::rand() % (maxValue - minValue + 1);
        
        int shift = (std::rand() % (2 * randomShiftMS + 1)) - randomShiftMS;
        int waitTime = intervalMS + shift;
        
        std::string nowStr = std::to_string(std::time(nullptr));

        std::string message = std::to_string(sensorValue) +"|" + nowStr + "\n";
        int written = comPort.write(message);
        
        if (written > 0) {
            std::cout << "Sent: " << message << " (wait: " << waitTime << " ms)" << std::endl;
        } else {
            std::cerr << "Error writing to port!" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    }

    return 0;
}