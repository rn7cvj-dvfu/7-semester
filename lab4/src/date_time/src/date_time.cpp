#include "date_time.hpp"
#include <sstream>
#include <iomanip>

DateTime::DateTime() {
    timestamp = std::time(nullptr);
}

DateTime::DateTime(std::time_t timestamp) : timestamp(timestamp) {}

DateTime::DateTime(const std::string& time_str) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    timestamp = std::mktime(&tm);
}

std::string DateTime::toString() const {
    std::tm* tm_info = std::localtime(&timestamp);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buffer);
}

std::time_t DateTime::getTimestamp() const {
    return timestamp;
}

void DateTime::setTimestamp(std::time_t timestamp) {
    this->timestamp = timestamp;
}

bool DateTime::operator<=(const DateTime& other) const {
    return timestamp <= other.timestamp;
}

bool DateTime::operator>=(const DateTime& other) const {
    return timestamp >= other.timestamp;
}

bool DateTime::operator<(const DateTime& other) const {
    return timestamp < other.timestamp;
}

bool DateTime::operator>(const DateTime& other) const {
    return timestamp > other.timestamp;
}

bool DateTime::operator==(const DateTime& other) const {
    return timestamp == other.timestamp;
}

std::time_t DateTime::operator-(const DateTime& other) const {
    return timestamp - other.timestamp;
}
