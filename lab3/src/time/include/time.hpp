
#include <string>

namespace Time{

    unsigned long long getCurrentTimeMillis();

    std::string getCurrentTimeString(std::string format = "%Y-%m-%d %H:%M:%S:%MS");
}

