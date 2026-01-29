#include "apihandler.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QNetworkRequest>

ApiHandler::ApiHandler(const QString &serverUrl, QObject *parent)
    : QObject(parent), serverUrl(serverUrl)
{
    networkManager = new QNetworkAccessManager(this);
}

void ApiHandler::requestCurrentTemperature()
{
    QUrl url(serverUrl + "/api/current");
    QNetworkRequest request(url);

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ApiHandler::onCurrentTempReplyFinished);
}

void ApiHandler::requestStatistics(const QDateTime &startTime, const QDateTime &endTime, const QString &period)
{
    QUrl url(serverUrl + "/api/stats");
    QUrlQuery query;
    query.addQueryItem("start", QString::number(startTime.toSecsSinceEpoch()));
    query.addQueryItem("end", QString::number(endTime.toSecsSinceEpoch()));
    query.addQueryItem("period", period);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ApiHandler::onStatsReplyFinished);
}

void ApiHandler::onCurrentTempReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        if (obj.contains("temperature") && obj.contains("timestamp"))
        {
            double temperature = obj["temperature"].toInt();
            QString timestampStr = obj["timestamp"].toString();
            QDateTime timestamp = parseTimestamp(timestampStr);

            emit currentTemperatureReceived(temperature, timestamp);
        }
    }
    else
    {
        emit errorOccurred("Failed to fetch current temperature: " + reply->errorString());
    }

    reply->deleteLater();
}

void ApiHandler::onStatsReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();

        // Проверяем первый элемент для определения типа данных
        if (!array.isEmpty())
        {
            QJsonObject firstObj = array[0].toObject();

            if (firstObj.contains("avg_temperature"))
            {
                // Агрегированные данные
                QVector<AggregatedTemperature> aggData;
                for (const QJsonValue &val : array)
                {
                    QJsonObject obj = val.toObject();
                    AggregatedTemperature agg;
                    agg.periodStart = parseTimestamp(obj["period_start"].toString());
                    agg.periodEnd = parseTimestamp(obj["period_end"].toString());
                    agg.avgTemperature = obj["avg_temperature"].toDouble();
                    agg.count = obj["count"].toInt();
                    aggData.append(agg);
                }
                emit aggregatedDataReceived(aggData);
            }
            else if (firstObj.contains("temperature"))
            {
                // Все измерения
                QVector<TemperatureMeasurement> measurements;
                for (const QJsonValue &val : array)
                {
                    QJsonObject obj = val.toObject();
                    TemperatureMeasurement meas;
                    meas.timestamp = parseTimestamp(obj["timestamp"].toString());
                    meas.temperature = obj["temperature"].toInt();
                    measurements.append(meas);
                }
                emit measurementsReceived(measurements);
            }
        }
    }
    else
    {
        emit errorOccurred("Failed to fetch statistics: " + reply->errorString());
    }

    reply->deleteLater();
}

QDateTime ApiHandler::parseTimestamp(const QString &timestampStr)
{
    // Формат: "YYYY-MM-DD HH:MM:SS"
    return QDateTime::fromString(timestampStr, "yyyy-MM-dd HH:mm:ss");
}
