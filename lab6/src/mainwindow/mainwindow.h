#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

#if QT_VERSION_MAJOR == 5
#  include <QtCharts/QChartGlobal>
#  ifndef QT_CHARTS_USE_NAMESPACE
#    define QT_CHARTS_USE_NAMESPACE using namespace QtCharts;
#  endif
QT_CHARTS_USE_NAMESPACE
#endif
#include "../apihandler/apihandler.h"

/**
 * @brief Главное окно приложения
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRefreshClicked();
    void onAutoRefreshToggled(bool checked);
    void onPeriodChanged(int index);
    void onViewTypeChanged(int index);

    void updateCurrentTemperature(double temperature, const QDateTime &timestamp);
    void updateChart(const QVector<TemperatureMeasurement> &measurements);
    void updateChartAggregated(const QVector<AggregatedTemperature> &data);
    void updateTable(const QVector<TemperatureMeasurement> &measurements);
    void updateTableAggregated(const QVector<AggregatedTemperature> &data);
    void showError(const QString &error);

    void refreshData();

private:
    void setupUi();
    void connectSignals();

    // API
    ApiHandler *apiHandler;

    // UI компоненты
    QLabel *currentTempLabel;
    QLabel *timestampLabel;
    QComboBox *periodComboBox;
    QComboBox *viewTypeComboBox;
    QPushButton *refreshButton;
    QPushButton *autoRefreshButton;

    QChartView *chartView;
    QChart *chart;
    QLineSeries *series;
    QDateTimeAxis *axisX;
    QValueAxis *axisY;

    QTableWidget *tableWidget;

    // Таймер автообновления
    QTimer *autoRefreshTimer;

    // Текущие настройки
    QString currentPeriod;
    QString currentViewType;
};

#endif // MAINWINDOW_H
