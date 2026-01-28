#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <atomic>
#include <csignal>

#include <virtual_com_port.hpp>
#include <date_time.hpp>
#include <database.hpp>
#include <http_server.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

// Глобальные переменные для остановки сервера
std::atomic<bool> g_running(true);
Database *g_database = nullptr;

void signalHandler(int signal)
{
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
}

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
std::string aggregatedDataToJson(const std::vector<AggregatedData> &data)
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
std::string measurementsToJson(const std::vector<Measurement> &measurements)
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
void handleCurrentTemperature(const HttpRequest &request, HttpResponse &response)
{
    Measurement last_measurement(DateTime(), 0);

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
void handleStatistics(const HttpRequest &request, HttpResponse &response)
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
void handleIndex(const HttpRequest &request, HttpResponse &response)
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
void sensorReaderThread(VirtualCOM::VirtualComPort &com_port, Database &db)
{
    while (g_running)
    {
        std::string data = com_port.read();

        if (data.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

                Measurement measurement(DateTime(timestamp), temperature);

                if (db.addMeasurement(measurement))
                {
                    std::cout << "Saved: " << measurement.timestamp.toString()
                              << " | " << measurement.temperature << "°C" << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing sensor data: " << e.what() << std::endl;
            }
        }
    }
}

/**
 * @brief Поток очистки старых данных
 */
void cleanupThread(Database &db)
{
    while (g_running)
    {
        // Спим 1 час
        for (int i = 0; i < 3600 && g_running; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (!g_running)
            break;

        // Удаляем данные старше 30 дней
        DateTime cutoff_time(std::time(nullptr) - (30 * 86400));
        int deleted = db.cleanOldData(cutoff_time);

        if (deleted > 0)
        {
            std::cout << "Cleaned " << deleted << " old measurements" << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Установка обработчиков сигналов
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <comPortName> <databasePath> <httpPort>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /tmp/vcom0 temperature.db 8080" << std::endl;
        return 1;
    }

    std::string com_port_name = argv[1];
    std::string db_path = argv[2];
    int http_port = std::stoi(argv[3]);

    // Создать директории для БД если нужно
    std::filesystem::create_directories(std::filesystem::path(db_path).parent_path());

    // Инициализация базы данных
    Database db(db_path);
    g_database = &db;

    if (!db.isOpen())
    {
        std::cerr << "Failed to open database: " << db_path << std::endl;
        return 1;
    }

    if (!db.initialize())
    {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    std::cout << "Database initialized: " << db_path << std::endl;

    // Открытие COM порта
    VirtualCOM::VirtualComPort com_port(com_port_name);

    if (!com_port.isOpen())
    {
        std::cerr << "Failed to open COM port: " << com_port_name << std::endl;
        return 1;
    }

    std::cout << "COM port opened: " << com_port_name << std::endl;

    // Запуск HTTP сервера
    HttpServer server(http_port);

    server.registerHandler("/", handleIndex);
    server.registerHandler("/api/current", handleCurrentTemperature);
    server.registerHandler("/api/stats", handleStatistics);

    if (!server.start())
    {
        std::cerr << "Failed to start HTTP server on port " << http_port << std::endl;
        return 1;
    }

    std::cout << "HTTP server started on port " << http_port << std::endl;
    std::cout << "Access web interface at http://localhost:" << http_port << "/" << std::endl;

    // Запуск потоков
    std::thread sensor_thread(sensorReaderThread, std::ref(com_port), std::ref(db));
    std::thread cleanup_thread_obj(cleanupThread, std::ref(db));

    // Ожидание завершения
    while (g_running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Остановка сервера
    server.stop();

    // Ожидание завершения потоков
    if (sensor_thread.joinable())
    {
        sensor_thread.join();
    }

    if (cleanup_thread_obj.joinable())
    {
        cleanup_thread_obj.join();
    }

    std::cout << "Server stopped" << std::endl;

    return 0;
}
