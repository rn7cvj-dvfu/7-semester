
#include <string>

namespace Time{

    unsigned long long GetCurrentTimeMillis();

    std::string GetCurrentTimeString(std::string format = "%Y-%m-%d %H:%M:%S:%MS");
}

