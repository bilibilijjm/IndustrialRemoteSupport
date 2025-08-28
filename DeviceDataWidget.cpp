#include "DeviceDataWidget.h"
#include "ui_DeviceDataWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QChartView>

using namespace QtCharts;

DeviceDataWidget::DeviceDataWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeviceDataWidget)
    , temperatureChart(new QChart())
    , pressureChart(new QChart())
    , temperatureSeries(new QLineSeries())
    , pressureSeries(new QLineSeries())
    , dataTimer(new QTimer(this))
    , chartTimer(new QTimer(this))
    , timeCounter(0)
    , currentTemperature(0.0)
    , currentPressure(0.0)
    , currentStatus("正常")
{
    ui->setupUi(this);
    setupUI();
    setupCharts();
#ifdef SIMULATION_MODE
    setupSimulatedData();
    qDebug() << "运行在模拟模式：使用模拟设备数据";
#else
    // 实际的串口初始化代码
    setupSerialPort();// 需要其他成员实现
#endif

    connect(dataTimer, &QTimer::timeout, this, &DeviceDataWidget::fetchData);
    connect(chartTimer, &QTimer::timeout, this, &DeviceDataWidget::updateCharts);
}

DeviceDataWidget::~DeviceDataWidget()
{
#ifdef SIMULATION_MODE
    if (simulationTimer) {
        simulationTimer->stop();
        delete simulationTimer;
    }
#else
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
        delete serialPort;
    }
#endif
    delete ui;
}

void DeviceDataWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 实时数据显示
    QHBoxLayout *dataLayout = new QHBoxLayout();

    QLabel *tempLabel = new QLabel(tr("温度: -- °C"), this);
    tempLabel->setObjectName("temperatureLabel");
    tempLabel->setStyleSheet("font-size: 14px; color: #4ec9b0;");

    QLabel *pressureLabel = new QLabel(tr("压力: -- kPa"), this);
    pressureLabel->setObjectName("pressureLabel");
    pressureLabel->setStyleSheet("font-size: 14px; color: #569cd6;");

    QLabel *statusLabel = new QLabel(tr("状态: --"), this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setStyleSheet("font-size: 14px; color: #ce9178;");

    dataLayout->addWidget(tempLabel);
    dataLayout->addWidget(pressureLabel);
    dataLayout->addWidget(statusLabel);
    dataLayout->addStretch();

    // 控制按钮
    QPushButton *startBtn = new QPushButton(tr("开始监控"), this);
    connect(startBtn, &QPushButton::clicked, this, &DeviceDataWidget::startMonitoring);

    QPushButton *stopBtn = new QPushButton(tr("停止监控"), this);
    connect(stopBtn, &QPushButton::clicked, this, &DeviceDataWidget::stopMonitoring);

    dataLayout->addWidget(startBtn);
    dataLayout->addWidget(stopBtn);

    mainLayout->addLayout(dataLayout);

    // 图表区域
    QHBoxLayout *chartLayout = new QHBoxLayout();

    QChartView *tempChartView = new QChartView(temperatureChart);
    tempChartView->setRenderHint(QPainter::Antialiasing);
    tempChartView->setMinimumSize(300, 200);

    QChartView *pressureChartView = new QChartView(pressureChart);
    pressureChartView->setRenderHint(QPainter::Antialiasing);
    pressureChartView->setMinimumSize(300, 200);

    chartLayout->addWidget(tempChartView);
    chartLayout->addWidget(pressureChartView);

    mainLayout->addLayout(chartLayout);
    setLayout(mainLayout);
}

void DeviceDataWidget::setupCharts()
{
    // 温度图表设置
    temperatureChart->addSeries(temperatureSeries);
    temperatureChart->setTitle(tr("温度监控"));
    temperatureChart->setTheme(QChart::ChartThemeDark);

    axisX = new QValueAxis();
    axisX->setRange(0, 100);
    axisX->setLabelFormat("%d");
    axisX->setTitleText(tr("时间(s)"));

    axisYTemperature = new QValueAxis();
    axisYTemperature->setRange(0, 100);
    axisYTemperature->setTitleText(tr("温度(°C)"));

    temperatureChart->addAxis(axisX, Qt::AlignBottom);
    temperatureChart->addAxis(axisYTemperature, Qt::AlignLeft);
    temperatureSeries->attachAxis(axisX);
    temperatureSeries->attachAxis(axisYTemperature);

    // 压力图表设置
    pressureChart->addSeries(pressureSeries);
    pressureChart->setTitle(tr("压力监控"));
    pressureChart->setTheme(QChart::ChartThemeDark);

    axisYPressure = new QValueAxis();
    axisYPressure->setRange(0, 1000);
    axisYPressure->setTitleText(tr("压力(kPa)"));

    pressureChart->addAxis(axisX, Qt::AlignBottom);
    pressureChart->addAxis(axisYPressure, Qt::AlignLeft);
    pressureSeries->attachAxis(axisX);
    pressureSeries->attachAxis(axisYPressure);
}

void DeviceDataWidget::setupSimulatedData()
{
    // 创建模拟数据定时器
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, [this]() {
        QJsonObject simulatedData;
        simulatedData["temperature"] = 20 + (qrand() % 150) / 10.0; // 20-35°C
        simulatedData["pressure"] = 90 + (qrand() % 200) / 10.0;    // 90-110 kPa
        simulatedData["vibration"] = (qrand() % 500) / 100.0;       // 0-5 mm/s
        simulatedData["current"] = 5 + (qrand() % 100) / 10.0;      // 5-15 A
        simulatedData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        updateDeviceData(simulatedData);
    });

    simulationTimer->start(2000); // 每2秒更新一次模拟数据
}

#ifndef SIMULATION_MODE
void DeviceDataWidget::setupSerialPort()
{
    // 实际的串口初始化代码（由其他成员实现）
    serialPort = new QSerialPort(this);
    // 这里将是实际的串口配置代码
}
#endif

void DeviceDataWidget::updateDeviceData(const QJsonObject &data)
{
    // 更新设备数据
    currentTemperature = data.value("temperature").toDouble();
    currentPressure = data.value("pressure").toDouble();
    currentStatus = data.value("status").toString("正常");

    // 更新UI
    QLabel *tempLabel = findChild<QLabel*>("temperatureLabel");
    QLabel *pressureLabel = findChild<QLabel*>("pressureLabel");
    QLabel *statusLabel = findChild<QLabel*>("statusLabel");

    if (tempLabel) tempLabel->setText(tr("温度: %1 °C").arg(currentTemperature, 0, 'f', 1));
    if (pressureLabel) pressureLabel->setText(tr("压力: %1 kPa").arg(currentPressure, 0, 'f', 1));
    if (statusLabel) statusLabel->setText(tr("状态: %1").arg(currentStatus));

    // 更新图表数据
        temperatureSeries->append(timeCounter, currentTemperature);
        pressureSeries->append(timeCounter, currentPressure);

        timeCounter++;
        if (timeCounter > 100) {
            axisX->setRange(timeCounter - 100, timeCounter);
        }
    }

    void DeviceDataWidget::fetchData()
    {
        emit dataRequested(); // 请求数据更新
    }

    void DeviceDataWidget::updateCharts()
    {
        // 自动调整Y轴范围
        QVector<QPointF> tempPoints = temperatureSeries->pointsVector();
        if (!tempPoints.isEmpty()) {
            double minTemp = 0, maxTemp = 100;
            for (const auto &point : tempPoints) {
                minTemp = qMin(minTemp, point.y());
                maxTemp = qMax(maxTemp, point.y());
            }
            axisYTemperature->setRange(minTemp - 5, maxTemp + 5);
        }

        QVector<QPointF> pressurePoints = pressureSeries->pointsVector();
        if (!pressurePoints.isEmpty()) {
            double minPressure = 0, maxPressure = 1000;
            for (const auto &point : pressurePoints) {
                minPressure = qMin(minPressure, point.y());
                maxPressure = qMax(maxPressure, point.y());
            }
            axisYPressure->setRange(minPressure - 50, maxPressure + 50);
        }
    }

    void DeviceDataWidget::startMonitoring()
    {
        dataTimer->start(1000); // 每秒获取一次数据
        chartTimer->start(5000); // 每5秒更新一次图表
    }

    void DeviceDataWidget::stopMonitoring()
    {
        dataTimer->stop();
        chartTimer->stop();
    }
