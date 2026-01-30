#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QDateTime>
#include <QVector>

/**
 * @brief Структура для хранения измерения температуры
 */
struct TemperatureMeasurement
{
    QDateTime timestamp;
    double temperature;

    TemperatureMeasurement() : temperature(0.0) {}
    TemperatureMeasurement(const QDateTime &ts, double temp)
        : timestamp(ts), temperature(temp) {}
};

/**
 * @brief Структура для агрегированных данных
 */
struct AggregatedTemperature
{
    QDateTime periodStart;
    QDateTime periodEnd;
    double avgTemperature;
    int count;

    AggregatedTemperature() : avgTemperature(0.0), count(0) {}
    AggregatedTemperature(const QDateTime &start, const QDateTime &end, double avg, int cnt)
        : periodStart(start), periodEnd(end), avgTemperature(avg), count(cnt) {}
};

/**
 * @brief Класс для работы с HTTP API сервера температуры
 */
class ApiHandler : public QObject
{
    Q_OBJECT

public:
    explicit ApiHandler(const QString &serverUrl, QObject *parent = nullptr);

    /**
     * @brief Запросить текущую температуру
     */
    void requestCurrentTemperature();

    /**
     * @brief Запросить статистику
     * @param startTime Начало периода
     * @param endTime Конец периода
     * @param period Тип периода: "all", "hourly", "daily"
     */
    void requestStatistics(const QDateTime &startTime, const QDateTime &endTime, const QString &period);

signals:
    /**
     * @brief Сигнал получения текущей температуры
     */
    void currentTemperatureReceived(double temperature, const QDateTime &timestamp);

    /**
     * @brief Сигнал получения всех измерений
     */
    void measurementsReceived(const QVector<TemperatureMeasurement> &measurements);

    /**
     * @brief Сигнал получения агрегированных данных
     */
    void aggregatedDataReceived(const QVector<AggregatedTemperature> &data);

    /**
     * @brief Сигнал ошибки
     */
    void errorOccurred(const QString &error);

private slots:
    void onCurrentTempReplyFinished();
    void onStatsReplyFinished();

private:
    QNetworkAccessManager *networkManager;
    QString serverUrl;

    QDateTime parseTimestamp(const QString &timestampStr);
};

#endif // APIHANDLER_H
