#pragma once

#include <string>
#include <vector>
#include <memory>
#include <date_time.hpp>
#include <sqlite3.h>

namespace Database
{

/**
 * @brief Структура для хранения записи измерения
 */
struct Measurement
{
    int id;
    DateTime timestamp;
    int temperature;

    Measurement(int id, const DateTime &timestamp, int temperature)
        : id(id), timestamp(timestamp), temperature(temperature) {}

    Measurement(const DateTime &timestamp, int temperature)
        : id(-1), timestamp(timestamp), temperature(temperature) {}
};

/**
 * @brief Структура для хранения агрегированной статистики
 */
struct AggregatedData
{
    DateTime period_start;
    DateTime period_end;
    double avg_temperature;
    int count;

    AggregatedData(const DateTime &start, const DateTime &end, double avg, int cnt)
        : period_start(start), period_end(end), avg_temperature(avg), count(cnt) {}
};

/**
 * @brief Класс для работы с базой данных измерений температуры
 */
class Database
{
public:
    /**
     * @brief Конструктор
     * @param db_path Путь к файлу базы данных
     */
    explicit Database(const std::string &db_path);

    /**
     * @brief Деструктор
     */
    ~Database();

    /**
     * @brief Инициализация структуры БД
     * @return true если успешно
     */
    bool initialize();

    /**
     * @brief Добавить измерение
     * @param measurement Измерение
     * @return true если успешно
     */
    bool addMeasurement(const Measurement &measurement);

    /**
     * @brief Получить последнее измерение
     * @param measurement Выходное измерение
     * @return true если успешно
     */
    bool getLastMeasurement(Measurement &measurement);

    /**
     * @brief Получить измерения за период
     * @param start_time Начало периода
     * @param end_time Конец периода
     * @return Вектор измерений
     */
    std::vector<Measurement> getMeasurements(const DateTime &start_time, const DateTime &end_time);

    /**
     * @brief Получить среднюю температуру за период
     * @param start_time Начало периода
     * @param end_time Конец периода
     * @return Средняя температура (или -999 если нет данных)
     */
    double getAverageTemperature(const DateTime &start_time, const DateTime &end_time);

    /**
     * @brief Получить почасовую статистику за период
     * @param start_time Начало периода
     * @param end_time Конец периода
     * @return Вектор агрегированных данных
     */
    std::vector<AggregatedData> getHourlyStats(const DateTime &start_time, const DateTime &end_time);

    /**
     * @brief Получить дневную статистику за период
     * @param start_time Начало периода
     * @param end_time Конец периода
     * @return Вектор агрегированных данных
     */
    std::vector<AggregatedData> getDailyStats(const DateTime &start_time, const DateTime &end_time);

    /**
     * @brief Удалить старые данные
     * @param older_than Удалить данные старше этого времени
     * @return Количество удаленных записей
     */
    int cleanOldData(const DateTime &older_than);

    /**
     * @brief Проверка открытия БД
     */
    bool isOpen() const { return db_ != nullptr; }

private:
    sqlite3 *db_;
    std::string db_path_;

    bool executeSQL(const std::string &sql);
};

} // namespace Database
