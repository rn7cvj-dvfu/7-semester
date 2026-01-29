#include "database.hpp"
#include <iostream>
#include <sstream>

namespace Database
{

Database::Database(const std::string &db_path)
    : db_(nullptr), db_path_(db_path)
{

    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        db_ = nullptr;
    }
}

Database::~Database()
{
    if (db_)
    {
        sqlite3_close(db_);
    }
}

bool Database::initialize()
{
    if (!db_)
        return false;

    const char *create_table_sql = R"(
        CREATE TABLE IF NOT EXISTS measurements (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            temperature INTEGER NOT NULL
        );
        
        CREATE INDEX IF NOT EXISTS idx_timestamp ON measurements(timestamp);
    )";

    return executeSQL(create_table_sql);
}

bool Database::executeSQL(const std::string &sql)
{
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool Database::addMeasurement(const Measurement &measurement)
{
    if (!db_)
        return false;

    const char *sql = "INSERT INTO measurements (timestamp, temperature) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, measurement.timestamp.getTimestamp());
    sqlite3_bind_int(stmt, 2, measurement.temperature);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool Database::getLastMeasurement(Measurement &measurement)
{
    if (!db_)
        return false;

    const char *sql = "SELECT id, timestamp, temperature FROM measurements ORDER BY timestamp DESC LIMIT 1;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        std::time_t timestamp = sqlite3_column_int64(stmt, 1);
        int temperature = sqlite3_column_int(stmt, 2);

        measurement = Measurement(id, DateTime(timestamp), temperature);
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

std::vector<Measurement> Database::getMeasurements(const DateTime &start_time, const DateTime &end_time)
{
    std::vector<Measurement> measurements;
    if (!db_)
        return measurements;

    const char *sql = "SELECT id, timestamp, temperature FROM measurements WHERE timestamp >= ? AND timestamp <= ? ORDER BY timestamp ASC;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        return measurements;
    }

    sqlite3_bind_int64(stmt, 1, start_time.getTimestamp());
    sqlite3_bind_int64(stmt, 2, end_time.getTimestamp());

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        std::time_t timestamp = sqlite3_column_int64(stmt, 1);
        int temperature = sqlite3_column_int(stmt, 2);

        measurements.push_back(Measurement(id, DateTime(timestamp), temperature));
    }

    sqlite3_finalize(stmt);
    return measurements;
}

double Database::getAverageTemperature(const DateTime &start_time, const DateTime &end_time)
{
    if (!db_)
        return -999.0;

    const char *sql = "SELECT AVG(temperature) FROM measurements WHERE timestamp >= ? AND timestamp <= ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        return -999.0;
    }

    sqlite3_bind_int64(stmt, 1, start_time.getTimestamp());
    sqlite3_bind_int64(stmt, 2, end_time.getTimestamp());

    double avg = -999.0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL)
        {
            avg = sqlite3_column_double(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return avg;
}

std::vector<AggregatedData> Database::getHourlyStats(const DateTime &start_time, const DateTime &end_time)
{
    std::vector<AggregatedData> stats;
    if (!db_)
        return stats;

    const char *sql = R"(
        SELECT 
            (timestamp / 3600) * 3600 as hour_start,
            AVG(temperature) as avg_temp,
            COUNT(*) as cnt
        FROM measurements 
        WHERE timestamp >= ? AND timestamp <= ?
        GROUP BY hour_start
        ORDER BY hour_start ASC;
    )";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        return stats;
    }

    sqlite3_bind_int64(stmt, 1, start_time.getTimestamp());
    sqlite3_bind_int64(stmt, 2, end_time.getTimestamp());

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::time_t hour_start = sqlite3_column_int64(stmt, 0);
        double avg_temp = sqlite3_column_double(stmt, 1);
        int count = sqlite3_column_int(stmt, 2);

        DateTime period_start(hour_start);
        DateTime period_end(hour_start + 3600);

        stats.push_back(AggregatedData(period_start, period_end, avg_temp, count));
    }

    sqlite3_finalize(stmt);
    return stats;
}

std::vector<AggregatedData> Database::getDailyStats(const DateTime &start_time, const DateTime &end_time)
{
    std::vector<AggregatedData> stats;
    if (!db_)
        return stats;

    const char *sql = R"(
        SELECT 
            (timestamp / 86400) * 86400 as day_start,
            AVG(temperature) as avg_temp,
            COUNT(*) as cnt
        FROM measurements 
        WHERE timestamp >= ? AND timestamp <= ?
        GROUP BY day_start
        ORDER BY day_start ASC;
    )";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        return stats;
    }

    sqlite3_bind_int64(stmt, 1, start_time.getTimestamp());
    sqlite3_bind_int64(stmt, 2, end_time.getTimestamp());

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::time_t day_start = sqlite3_column_int64(stmt, 0);
        double avg_temp = sqlite3_column_double(stmt, 1);
        int count = sqlite3_column_int(stmt, 2);

        DateTime period_start(day_start);
        DateTime period_end(day_start + 86400);

        stats.push_back(AggregatedData(period_start, period_end, avg_temp, count));
    }

    sqlite3_finalize(stmt);
    return stats;
}

int Database::cleanOldData(const DateTime &older_than)
{
    if (!db_)
        return 0;

    const char *sql = "DELETE FROM measurements WHERE timestamp < ?;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        return 0;
    }

    sqlite3_bind_int64(stmt, 1, older_than.getTimestamp());

    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db_);

    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? changes : 0;
}

} // namespace Database
