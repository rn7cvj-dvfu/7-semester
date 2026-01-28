

#include "time.hpp"
#include <windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Time {

    unsigned long long getCurrentTimeMillis() {
        // Получаем текущее время через chrono
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return static_cast<unsigned long long>(millis);
    }

    std::string getCurrentTimeString(std::string format) {
        // Получаем текущее системное время
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        // Форматируем время
        std::ostringstream oss;
        
        // Простая замена формата (поддержка базовых спецификаторов)
        std::string result = format;
        
        // Замена %Y - год (4 цифры)
        size_t pos = result.find("%Y");
        if (pos != std::string::npos) {
            char year[5];
            sprintf_s(year, sizeof(year), "%04d", st.wYear);
            result.replace(pos, 2, year);
        }
        
        // Замена %m - месяц (2 цифры)
        pos = result.find("%m");
        if (pos != std::string::npos) {
            char month[3];
            sprintf_s(month, sizeof(month), "%02d", st.wMonth);
            result.replace(pos, 2, month);
        }
        
        // Замена %d - день (2 цифры)
        pos = result.find("%d");
        if (pos != std::string::npos) {
            char day[3];
            sprintf_s(day, sizeof(day), "%02d", st.wDay);
            result.replace(pos, 2, day);
        }
        
        // Замена %H - час (2 цифры, 24-часовой формат)
        pos = result.find("%H");
        if (pos != std::string::npos) {
            char hour[3];
            sprintf_s(hour, sizeof(hour), "%02d", st.wHour);
            result.replace(pos, 2, hour);
        }
        
        // Замена %M - минута (2 цифры)
        pos = result.find("%M");
        if (pos != std::string::npos) {
            char minute[3];
            sprintf_s(minute, sizeof(minute), "%02d", st.wMinute);
            result.replace(pos, 2, minute);
        }
        
        // Замена %S - секунда (2 цифры)
        pos = result.find("%S");
        if (pos != std::string::npos) {
            char second[3];
            sprintf_s(second, sizeof(second), "%02d", st.wSecond);
            result.replace(pos, 2, second);
        }
        
        // Замена %MS - миллисекунды (3 цифры)
        pos = result.find("%MS");
        if (pos != std::string::npos) {
            char millis[4];
            sprintf_s(millis, sizeof(millis), "%03d", st.wMilliseconds);
            result.replace(pos, 3, millis);
        }
        
        return result;
    }

}

