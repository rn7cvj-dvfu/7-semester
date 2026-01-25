
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <filesystem>
#include <virtual_com_port.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <date_time.hpp>


const auto dayLifeTime = std::chrono::hours(24);
const auto monthLifeTime = std::chrono::hours(24 * 30);
const auto yearLifeTime = std::chrono::hours(24 * 365);


/**
 * @brief Структура для хранения записи лога
 * 
 * @param measurement_begin Время начала измерения
 * @param measurement_end Время окончания измерения
 * @param sensor_value Значение с датчика
 * @param have_data Флаг наличия данных (true - данные есть, false - sensor_value не имеет смысла)
 * 
 */
struct LogEntry {

    DateTime measurement_begin;
    DateTime measurement_end;

    int sensor_value;
    bool have_data;

    LogEntry(
        DateTime measurement_begin, 
        DateTime measurement_end, 
        int sensor_value , 
        bool have_data = true
    ) : measurement_begin(measurement_begin), measurement_end(measurement_end), sensor_value(sensor_value), have_data(have_data) {}
    
    std::string toString() const {
      
        std::ostringstream oss;
        oss << measurement_begin.toString() << " - " 
            << measurement_end.toString() << " | " 
            << (have_data ? std::to_string(sensor_value) : "No Data");

        return oss.str();
    }

    static LogEntry fromString(const std::string& str) {
    
        size_t delimiter_pos = str.find(' | ');


        std::string timestamps_part = str.substr(0, delimiter_pos);
        std::string value_part = str.substr(delimiter_pos + 3);

        bool have_data = true;
        int sensor_value = 0;

        if (value_part == "No Data") {
            have_data = false;
        } else {
            sensor_value = std::stoi(value_part);
        }

        size_t dash_pos = timestamps_part.find(" - ");
        if (dash_pos == std::string::npos) {
            throw std::invalid_argument("Invalid timestamp format");
        }

        std::string begin_str = timestamps_part.substr(0, dash_pos);
        std::string end_str = timestamps_part.substr(dash_pos + 3);

        DateTime begin_timestamp = DateTime(begin_str);
        DateTime end_timestamp = DateTime(end_str);

        return LogEntry(begin_timestamp, end_timestamp, sensor_value, have_data);
    }
};

std::vector<LogEntry> parseLogFile(const std::string& filename) {
    std::vector<LogEntry> entries;
    std::ifstream file(filename , std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Could not open log file: " << filename << std::endl;
        return entries;
    }   

    std::string line;
    while (std::getline(file, line)) {
        try {
            LogEntry entry = LogEntry::fromString(line);
            entries.push_back(entry);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing log entry: " << e.what() << std::endl;
        }
    }

    return entries;
}

std::vector<LogEntry> filterByLiveTime(
    const std::vector<LogEntry>& entries, 
    std::chrono::hours live_time
) {
    std::vector<LogEntry> filtered;

    std::time_t now = std::time(nullptr);
    
    for (const auto& entry : entries) {
        if (now - entry.measurement_end.getTimestamp() <= live_time.count() * 3600) {
            filtered.push_back(entry);
        }
    }
    return filtered;
}


void saveLogFile(const std::string& filename, const std::vector<LogEntry>& entries) {
    std::ofstream file(filename,  std::ios::trunc);

    if (!file.is_open()) {
        std::cerr << "Could not open log file for writing: " << filename << std::endl;
        return;
    }

    for (const auto& entry : entries) {
        file << entry.toString() << std::endl;
    }
}   

bool addToMeanHourLog( std::vector<LogEntry>& all_log_entries,  std::vector<LogEntry>& hour_log_entries ) {
    DateTime now;

    DateTime hour_begin = DateTime(now.getTimestamp() - (now.getTimestamp() % 3600));
    DateTime hour_end = DateTime(hour_begin.getTimestamp() + 3600);

    std::vector<LogEntry> same_hour_entries ={};

    auto it = all_log_entries.end();

    while (it != all_log_entries.begin()) {
        --it;
        auto entry_time = it->measurement_end;
        if (entry_time >= hour_begin && entry_time < hour_end) {
            same_hour_entries.push_back(*it);
            continue;
        } 
        break;
    }

    if (same_hour_entries.empty()){
        hour_log_entries.push_back(LogEntry(DateTime(hour_begin), DateTime(hour_end), 0, false));
        return true;
    }

    int mean_value = 0;
    for (const auto& entry : same_hour_entries) {
        mean_value += entry.sensor_value;
    }
    mean_value /= same_hour_entries.size();


    if (hour_log_entries.empty()) {
        hour_log_entries.push_back(LogEntry(DateTime(hour_begin), DateTime(hour_end), mean_value));
        return true;
    }

    LogEntry last_hour_log  = *( --hour_log_entries.end());

    if (last_hour_log_hour == hour_end){
        return false;
    }

    hour_log_entries.push_back(LogEntry(DateTime(hour_begin), DateTime(hour_end), mean_value));

    return true;
}

bool addToMeanDayLog( std::vector<LogEntry>& hour_log_entries,  std::vector<LogEntry>& day_log_entries ) {
    // Similar implementation to addToMeanHourLog but for daily aggregation
    return true;
}

int main(int argc, char* argv[]) {
    
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <comPortName> <allLogFile> <meanHourLogFile> <meanDayLogFile>" << std::endl;
        return 1;
    }

    std::string com_port_name = argv[1];
    std::string all_log_file_name = argv[2];
    std::string mean_hour_log_file_name = argv[3];
    std::string mean_day_log_file_name = argv[4];

    // Create directories if they don't exist
    std::filesystem::create_directories(std::filesystem::path(all_log_file_name).parent_path());
    std::filesystem::create_directories(std::filesystem::path(mean_hour_log_file_name).parent_path());
    std::filesystem::create_directories(std::filesystem::path(mean_day_log_file_name).parent_path());

    VirtualCOM::VirtualComPort com_port(com_port_name);

    if (!com_port.isOpen()) {
        std::cerr << "Не удалось открыть COM порт: " << com_port_name << std::endl;
        return 1;
    }

    std::vector<LogEntry> all_log_entries = parseLogFile(all_log_file_name);
    all_log_entries = filterByLiveTime(all_log_entries, dayLifeTime); 
    
    std::vector<LogEntry> hour_log_entries = parseLogFile(mean_hour_log_file_name);
    hour_log_entries = filterByLiveTime(hour_log_entries, monthLifeTime);

    std::vector<LogEntry> day_log_entries = parseLogFile(mean_day_log_file_name);
    day_log_entries = filterByLiveTime(day_log_entries, yearLifeTime);

    while(true){

        std::string data = com_port.read(1024  );

        std::vector<LogEntry> new_entries = {};

        std::istringstream ss(data);

        std::string token;
        while (std::getline(ss, token, '\n')) {
            try {
                
                std::string value_part;
                std::string time_part;

                size_t delimiter_pos = token.find('|');

                if (delimiter_pos == std::string::npos) {
                    std::cerr << "Invalid data format: " << token << std::endl;
                    continue;
                }

                value_part = token.substr(0, delimiter_pos);
                time_part = token.substr(delimiter_pos + 1);

                int sensor_value = std::stoi(value_part);
                std::time_t timestamp = static_cast<std::time_t>(std::stoll(time_part));

                LogEntry entry(DateTime(timestamp), DateTime(timestamp), sensor_value);
                new_entries.push_back(entry);

            } catch (const std::exception& e) {
                std::cerr << "Error parsing sensor value: " << e.what() << std::endl;
            }
        }
        
        all_log_entries.insert(all_log_entries.end(), new_entries.begin(), new_entries.end());

        saveLogFile(all_log_file_name, all_log_entries);

        if (addToMeanHourLog(all_log_entries, hour_log_entries)) {
            saveLogFile(mean_hour_log_file_name, hour_log_entries);
        }
        
    }


    return 0;
}