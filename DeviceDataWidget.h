#ifndef DEVICEDATAWIDGET_H
#define DEVICEDATAWIDGET_H

// 添加条件编译宏
#define SIMULATION_MODE  // 注释这行来禁用模拟模式

#ifdef SIMULATION_MODE
// 模拟模式 - 不需要QSerialPort
#else
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

#include <QWidget>
#include <QtCharts>
#include <QTimer>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class DeviceDataWidget; }
QT_END_NAMESPACE

class DeviceDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceDataWidget(QWidget *parent = nullptr);
    ~DeviceDataWidget();

signals:
    void dataRequested();

public slots:
    void updateDeviceData(const QJsonObject &data);
    void startMonitoring();
    void stopMonitoring();

private slots:
    void fetchData();
    void updateCharts();

private:
    void setupUI();
    void setupCharts();
    void setupSerialPort(); // 需要其他成员实现串口通信
    void setupSimulatedData();  // 模拟数据方法

    #ifdef SIMULATION_MODE
    // 模拟数据相关成员
    QTimer *simulationTimer;
    #else
    // 实际串口相关成员
    QSerialPort *serialPort;
    #endif

private:
    Ui::DeviceDataWidget *ui;
    QChart *temperatureChart;
    QChart *pressureChart;
    QLineSeries *temperatureSeries;
    QLineSeries *pressureSeries;
    QValueAxis *axisX;
    QValueAxis *axisYTemperature;
    QValueAxis *axisYPressure;

    QTimer *dataTimer;
    QTimer *chartTimer;
    int timeCounter;

    double currentTemperature;
    double currentPressure;
    QString currentStatus;
};

#endif // DEVICEDATAWIDGET_H
