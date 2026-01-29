#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <csignal>
#include <ctime>

#include <virtual_com_port.hpp>
#include <date_time.hpp>
#include <database.hpp>
#include <http_server.hpp>
#include <threads.hpp>

using namespace Threads;
namespace DB = Database;
namespace HS = HttpServer;

// Глобальные переменные для HTTP обработчиков
DB::Database *g_database = nullptr;

/**
 * @brief Парсинг query string параметров
 */
std::map<std::string, std::string> parseQueryString(const std::string &query)
{
    std::map<std::string, std::string> params;
    std::istringstream stream(query);
    std::string pair;

    while (std::getline(stream, pair, '&'))
    {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos)
        {
            std::string key = pair.substr(0, eq_pos);
            std::string value = pair.substr(eq_pos + 1);
            params[key] = value;
        }
    }

    return params;
}

/**
 * @brief Преобразование AggregatedData в JSON
 */
std::string aggregatedDataToJson(const std::vector<DB::AggregatedData> &data)
{
    std::ostringstream oss;
    oss << "[";

    for (size_t i = 0; i < data.size(); ++i)
    {
        if (i > 0)
            oss << ",";
        oss << "{";
        oss << "\"period_start\":\"" << data[i].period_start.toString() << "\",";
        oss << "\"period_end\":\"" << data[i].period_end.toString() << "\",";
        oss << "\"avg_temperature\":" << data[i].avg_temperature << ",";
        oss << "\"count\":" << data[i].count;
        oss << "}";
    }

    oss << "]";
    return oss.str();
}

/**
 * @brief Преобразование Measurement в JSON
 */
std::string measurementsToJson(const std::vector<DB::Measurement> &measurements)
{
    std::ostringstream oss;
    oss << "[";

    for (size_t i = 0; i < measurements.size(); ++i)
    {
        if (i > 0)
            oss << ",";
        oss << "{";
        oss << "\"id\":" << measurements[i].id << ",";
        oss << "\"timestamp\":\"" << measurements[i].timestamp.toString() << "\",";
        oss << "\"temperature\":" << measurements[i].temperature;
        oss << "}";
    }

    oss << "]";
    return oss.str();
}

/**
 * @brief Обработчик для получения текущей температуры
 */
void handleCurrentTemperature(const HS::HttpRequest &request, HS::HttpResponse &response)
{
    DB::Measurement last_measurement(DateTime(), 0);

    if (g_database->getLastMeasurement(last_measurement))
    {
        std::ostringstream json;
        json << "{";
        json << "\"timestamp\":\"" << last_measurement.timestamp.toString() << "\",";
        json << "\"temperature\":" << last_measurement.temperature;
        json << "}";

        response.setJson(json.str());
    }
    else
    {
        response.status_code = 404;
        response.status_message = "Not Found";
        response.setJson("{\"error\":\"No data available\"}");
    }
}

/**
 * @brief Обработчик для получения статистики
 */
void handleStatistics(const HS::HttpRequest &request, HS::HttpResponse &response)
{
    auto params = parseQueryString(request.query_string);

    // Параметры по умолчанию: последние 24 часа
    DateTime now;
    DateTime start_time(now.getTimestamp() - 86400);
    DateTime end_time = now;

    // Тип агрегации: all, hourly, daily
    std::string period_type = "all";

    if (params.count("start"))
    {
        try
        {
            start_time = DateTime(std::stoll(params["start"]));
        }
        catch (...)
        {
        }
    }

    if (params.count("end"))
    {
        try
        {
            end_time = DateTime(std::stoll(params["end"]));
        }
        catch (...)
        {
        }
    }

    if (params.count("period"))
    {
        period_type = params["period"];
    }

    std::string json_data;

    if (period_type == "hourly")
    {
        auto data = g_database->getHourlyStats(start_time, end_time);
        json_data = aggregatedDataToJson(data);
    }
    else if (period_type == "daily")
    {
        auto data = g_database->getDailyStats(start_time, end_time);
        json_data = aggregatedDataToJson(data);
    }
    else
    {
        auto measurements = g_database->getMeasurements(start_time, end_time);
        json_data = measurementsToJson(measurements);
    }

    response.setJson(json_data);
}

/**
 * @brief Обработчик для главной страницы (отдает HTML файл)
 */
void handleIndex(const HS::HttpRequest &request, HS::HttpResponse &response)
{
    std::ifstream file("web/index.html");

    if (file.is_open())
    {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        response.setHtml(content);
    }
    else
    {
        response.status_code = 404;
        response.status_message = "Not Found";
        response.setPlainText("404 - index.html not found");
    }
}

/**
 * @brief Поток чтения данных с COM порта
 */
class SensorReaderThread : public Thread
{
public:
    SensorReaderThread(VirtualCOM::VirtualComPort &port, DB::Database &database)
        : _com_port(port), _db(database)
    {
    }

protected:
    int MainStart() override
    {
        std::cout << "[SENSOR] Sensor reader thread started" << std::endl;
        return 0;
    }

    void Main() override
    {
        while (true)
        {
            std::string data = _com_port.read();

            if (data.empty())
            {
                Thread::Sleep(0.1);  // 100 ms
                continue;
            }

            std::istringstream ss(data);
            std::string token;

            while (std::getline(ss, token, '\n'))
            {
                try
                {
                    size_t delimiter_pos = token.find('|');
                    if (delimiter_pos == std::string::npos)
                    {
                        continue;
                    }

                    std::string value_part = token.substr(0, delimiter_pos);
                    std::string time_part = token.substr(delimiter_pos + 1);

                    int temperature = std::stoi(value_part);
                    std::time_t timestamp = static_cast<std::time_t>(std::stoll(time_part));

                    DB::Measurement measurement(DateTime(timestamp), temperature);

                    if (_db.addMeasurement(measurement))
                    {
                        std::cout << "[SENSOR] Temperature recorded: " << temperature 
                                  << "°C at " << measurement.timestamp.toString() << std::endl;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "[SENSOR] Parse error: " << e.what() << std::endl;
                }
            }
            CancelPoint();
        }
    }

    void MainQuit() override
    {
        std::cout << "[SENSOR] Sensor reader thread stopped" << std::endl;
    }

private:
    VirtualCOM::VirtualComPort &_com_port;
    DB::Database &_db;
};

/**
 * @brief Поток очистки старых данных
 */
class CleanupThread : public Thread
{
public:
    CleanupThread(DB::Database &database) : _db(database) {}

protected:
    int MainStart() override
    {
        std::cout << "[CLEANUP] Cleanup thread started (runs every hour)" << std::endl;
        return 0;
    }

    void Main() override
    {
        const int SLEEP_INTERVAL = 3600;  // 1 hour in seconds
        int sleep_counter = 0;

        while (true)
        {
            // Спим 1 час с проверкой каждую секунду
            sleep_counter = 0;
            while (sleep_counter < SLEEP_INTERVAL)
            {
                Thread::Sleep(1.0);
                sleep_counter++;
            }

            // Удаляем данные старше 30 дней
            DateTime cutoff_time(std::time(nullptr) - (30 * 86400));
            int deleted = _db.cleanOldData(cutoff_time);

            if (deleted > 0)
            {
                std::cout << "[CLEANUP] Removed " << deleted << " old measurements" << std::endl;
            }

            CancelPoint();
        }
    }

    void MainQuit() override
    {
        std::cout << "[CLEANUP] Cleanup thread stopped" << std::endl;
    }

private:
    DB::Database &_db;
};


int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <comPortName> <databasePath> <httpPort>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /tmp/vcom0 /tmp/temperature.db 8080" << std::endl;
        return 1;
    }

    std::string com_port_name = argv[1];
    std::string db_path = argv[2];
    int http_port = std::stoi(argv[3]);

    std::filesystem::create_directories(std::filesystem::path(db_path).parent_path());

    DB::Database db(db_path);
    g_database = &db;

    if (!db.isOpen())
    {
        std::cerr << "ERROR: Failed to open database: " << db_path << std::endl;
        return 1;
    }

    if (!db.initialize())
    {
        std::cerr << "ERROR: Failed to initialize database" << std::endl;
        return 1;
    }

    std::cout << "Database initialized: " << db_path << std::endl;

    VirtualCOM::VirtualComPort com_port(com_port_name);

    if (!com_port.isOpen())
    {
        std::cerr << "ERROR: Failed to open COM port: " << com_port_name << std::endl;
        return 1;
    }

    std::cout << "COM port opened: " << com_port_name << std::endl;

    HS::HttpServer server(http_port);

    server.registerHandler("/", handleIndex);
    server.registerHandler("/api/current", handleCurrentTemperature);
    server.registerHandler("/api/stats", handleStatistics);

    if (!server.start())
    {
        std::cerr << "ERROR: Failed to start HTTP server on port " << http_port << std::endl;
        return 1;
    }

    std::cout << "HTTP server started on port " << http_port << std::endl;
    std::cout << "Access interface at http://localhost:" << http_port << "/" << std::endl;

    SensorReaderThread sensor_thread(com_port, db);
    CleanupThread cleanup_thread(db);

    if (sensor_thread.Start() != Threads::THREAD_SUCCESS)
    {
        std::cerr << "ERROR: Failed to start sensor reader thread" << std::endl;
        server.stop();
        return 1;
    }

    if (cleanup_thread.Start() != Threads::THREAD_SUCCESS)
    {
        std::cerr << "ERROR: Failed to start cleanup thread" << std::endl;
        sensor_thread.Stop();
        sensor_thread.Join();
        server.stop();
        return 1;
    }

    sensor_thread.WaitStartup();
    cleanup_thread.WaitStartup();

    std::string input;

    while (true)
    {
        std::getline(std::cin, input);
        
        if (std::cin.eof())
        {
            std::cout << "\nShutting down..." << std::endl;
            break;
        }
    }

    std::cout << "Stopping threads..." << std::endl;

    sensor_thread.Stop();
    cleanup_thread.Stop();

    sensor_thread.Join();
    cleanup_thread.Join();

    server.stop();

    std::cout << "Server stopped successfully" << std::endl;

    return 0;
}
