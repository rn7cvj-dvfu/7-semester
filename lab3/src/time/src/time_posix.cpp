#ifndef _WIN32

#include "time.hpp"
#include <sys/time.h>
#include <ctime>
#include <cstring>

namespace Time {

unsigned long long GetCurrentTimeMillis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    
    unsigned long long milliseconds = 
        static_cast<unsigned long long>(tv.tv_sec) * 1000ULL +
        static_cast<unsigned long long>(tv.tv_usec) / 1000ULL;
    
    return milliseconds;
}

std::string GetCurrentTimeString(std::string format) {
    // Получаем текущее время с миллисекундами
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    
    time_t now = tv.tv_sec;
    struct tm* timeinfo = localtime(&now);
    
    // Вычисляем миллисекунды
    int milliseconds = tv.tv_usec / 1000;
    
    // Сначала обрабатываем %MS
    std::string result = format;
    size_t pos = result.find("%MS");
    if (pos != std::string::npos) {
        char millis[4];
        snprintf(millis, sizeof(millis), "%03d", milliseconds);
        result.replace(pos, 3, millis);
    }
    
    // Затем форматируем остальное время согласно формату
    char buffer[128];
    strftime(buffer, sizeof(buffer), result.c_str(), timeinfo);
    
    return std::string(buffer);
}

} // namespace Time

#endif // !_WIN32
