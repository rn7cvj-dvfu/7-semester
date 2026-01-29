#include "mainwindow.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentPeriod("86400"), currentViewType("hourly")
{
    setupUi();

    // Создать API handler
    apiHandler = new ApiHandler("http://localhost:8080", this);
    connectSignals();

    // Таймер автообновления
    autoRefreshTimer = new QTimer(this);
    connect(autoRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshData);

    // Первоначальная загрузка данных
    refreshData();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    setWindowTitle("Temperature Monitor");
    setMinimumSize(1000, 700);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // === Текущая температура ===
    QGroupBox *currentTempGroup = new QGroupBox("Текущая температура");
    QVBoxLayout *tempLayout = new QVBoxLayout(currentTempGroup);

    currentTempLabel = new QLabel("--°C");
    QFont tempFont = currentTempLabel->font();
    tempFont.setPointSize(48);
    tempFont.setBold(true);
    currentTempLabel->setFont(tempFont);
    currentTempLabel->setAlignment(Qt::AlignCenter);

    timestampLabel = new QLabel("Загрузка...");
    timestampLabel->setAlignment(Qt::AlignCenter);

    tempLayout->addWidget(currentTempLabel);
    tempLayout->addWidget(timestampLabel);

    // === Панель управления ===
    QGroupBox *controlGroup = new QGroupBox("Настройки");
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    controlLayout->addWidget(new QLabel("Период:"));
    periodComboBox = new QComboBox();
    periodComboBox->addItem("Последний час", "3600");
    periodComboBox->addItem("Последние 6 часов", "21600");
    periodComboBox->addItem("Последние 24 часа", "86400");
    periodComboBox->addItem("Последние 3 дня", "259200");
    periodComboBox->setCurrentIndex(2);
    controlLayout->addWidget(periodComboBox);

    controlLayout->addWidget(new QLabel("Тип отображения:"));
    viewTypeComboBox = new QComboBox();
    viewTypeComboBox->addItem("Все измерения", "all");
    viewTypeComboBox->addItem("Среднее (час)", "hourly");
    viewTypeComboBox->addItem("Среднее (день)", "daily");
    viewTypeComboBox->setCurrentIndex(1);
    controlLayout->addWidget(viewTypeComboBox);

    refreshButton = new QPushButton("🔄 Обновить");
    controlLayout->addWidget(refreshButton);

    autoRefreshButton = new QPushButton("⏯ Авто");
    autoRefreshButton->setCheckable(true);
    autoRefreshButton->setChecked(true);
    controlLayout->addWidget(autoRefreshButton);

    controlLayout->addStretch();

    // === График ===
    QGroupBox *chartGroup = new QGroupBox("График температуры");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);

    chart = new QChart();
    chart->setTitle("Температура");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    series = new QLineSeries();
    chart->addSeries(series);

    axisX = new QDateTimeAxis();
    axisX->setFormat("dd.MM HH:mm");
    axisX->setTitleText("Время");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setTitleText("Температура (°C)");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);

    chartLayout->addWidget(chartView);

    // === Таблица ===
    QGroupBox *tableGroup = new QGroupBox("Последние измерения");
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(3);
    tableWidget->setHorizontalHeaderLabels({"Время", "Температура (°C)", "Значение"});
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    tableLayout->addWidget(tableWidget);

    // === Сборка ===
    mainLayout->addWidget(currentTempGroup);
    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(chartGroup, 1);
    mainLayout->addWidget(tableGroup, 1);

    setCentralWidget(centralWidget);

    // Применить стили
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 1px solid #cccccc;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QPushButton {
            padding: 5px 15px;
            border: 1px solid #cccccc;
            border-radius: 3px;
            background: white;
        }
        QPushButton:hover {
            background: #f0f0f0;
        }
        QPushButton:checked {
            background: #e0e0e0;
        }
    )");
}

void MainWindow::connectSignals()
{
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(autoRefreshButton, &QPushButton::toggled, this, &MainWindow::onAutoRefreshToggled);
    connect(periodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onPeriodChanged);
    connect(viewTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onViewTypeChanged);

    connect(apiHandler, &ApiHandler::currentTemperatureReceived, this, &MainWindow::updateCurrentTemperature);
    connect(apiHandler, &ApiHandler::measurementsReceived, this, &MainWindow::updateChart);
    connect(apiHandler, &ApiHandler::aggregatedDataReceived, this, &MainWindow::updateChartAggregated);
    connect(apiHandler, &ApiHandler::errorOccurred, this, &MainWindow::showError);
}

void MainWindow::onRefreshClicked()
{
    refreshData();
}

void MainWindow::onAutoRefreshToggled(bool checked)
{
    if (checked)
    {
        autoRefreshTimer->start(5000); // Обновлять каждые 5 секунд
        autoRefreshButton->setText("⏸ Авто");
    }
    else
    {
        autoRefreshTimer->stop();
        autoRefreshButton->setText("▶ Авто");
    }
}

void MainWindow::onPeriodChanged(int index)
{
    currentPeriod = periodComboBox->itemData(index).toString();
    refreshData();
}

void MainWindow::onViewTypeChanged(int index)
{
    currentViewType = viewTypeComboBox->itemData(index).toString();
    refreshData();
}

void MainWindow::updateCurrentTemperature(double temperature, const QDateTime &timestamp)
{
    currentTempLabel->setText(QString::number(temperature, 'f', 0) + "°C");
    timestampLabel->setText("Обновлено: " + timestamp.toString("dd.MM.yyyy HH:mm:ss"));
}

void MainWindow::updateChart(const QVector<TemperatureMeasurement> &measurements)
{
    series->clear();

    if (measurements.isEmpty())
    {
        return;
    }

    double minTemp = 1000, maxTemp = -1000;

    for (const auto &meas : measurements)
    {
        series->append(meas.timestamp.toMSecsSinceEpoch(), meas.temperature);
        minTemp = qMin(minTemp, meas.temperature);
        maxTemp = qMax(maxTemp, meas.temperature);
    }

    axisX->setRange(measurements.first().timestamp, measurements.last().timestamp);
    axisY->setRange(minTemp - 2, maxTemp + 2);

    updateTable(measurements);
}

void MainWindow::updateChartAggregated(const QVector<AggregatedTemperature> &data)
{
    series->clear();

    if (data.isEmpty())
    {
        return;
    }

    double minTemp = 1000, maxTemp = -1000;

    for (const auto &agg : data)
    {
        series->append(agg.periodStart.toMSecsSinceEpoch(), agg.avgTemperature);
        minTemp = qMin(minTemp, agg.avgTemperature);
        maxTemp = qMax(maxTemp, agg.avgTemperature);
    }

    axisX->setRange(data.first().periodStart, data.last().periodEnd);
    axisY->setRange(minTemp - 2, maxTemp + 2);

    updateTableAggregated(data);
}

void MainWindow::updateTable(const QVector<TemperatureMeasurement> &measurements)
{
    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels({"Время", "Температура (°C)"});

    // Показываем последние 20 записей
    int startIdx = qMax(0, measurements.size() - 20);
    for (int i = measurements.size() - 1; i >= startIdx; --i)
    {
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);

        tableWidget->setItem(row, 0, new QTableWidgetItem(measurements[i].timestamp.toString("dd.MM.yyyy HH:mm:ss")));
        tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(measurements[i].temperature, 'f', 1)));
    }
}

void MainWindow::updateTableAggregated(const QVector<AggregatedTemperature> &data)
{
    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"Начало периода", "Конец периода", "Средняя температура (°C)", "Кол-во"});

    // Показываем последние 20 записей
    int startIdx = qMax(0, data.size() - 20);
    for (int i = data.size() - 1; i >= startIdx; --i)
    {
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);

        tableWidget->setItem(row, 0, new QTableWidgetItem(data[i].periodStart.toString("dd.MM.yyyy HH:mm")));
        tableWidget->setItem(row, 1, new QTableWidgetItem(data[i].periodEnd.toString("dd.MM.yyyy HH:mm")));
        tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(data[i].avgTemperature, 'f', 2)));
        tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(data[i].count)));
    }
}

void MainWindow::showError(const QString &error)
{
    QMessageBox::warning(this, "Ошибка", error);
}

void MainWindow::refreshData()
{
    // Запросить текущую температуру
    apiHandler->requestCurrentTemperature();

    // Запросить статистику
    QDateTime endTime = QDateTime::currentDateTime();
    QDateTime startTime = endTime.addSecs(-currentPeriod.toInt());
    apiHandler->requestStatistics(startTime, endTime, currentViewType);
}
