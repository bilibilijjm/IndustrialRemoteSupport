#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "VideoWidget.h"
#include "DeviceDataWidget.h"
#include "ChatWidget.h"
#include "TicketManager.h"

// 由其他成员开发的Socket客户端，此处需要替换为实际头文件
#include "socketclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginSuccess(const QString &username);
    void onCreateTicket();
    void onJoinTicket();
    void onTicketCreated(const QString &ticketId);
    void onVideoStartStop(bool started);
    void onAudioStartStop(bool started);
    void onScreenShareStartStop(bool started);
    void onSendMessage(const QString &message);
    void onDeviceDataReceived(const QJsonObject &data);
    void onRecordingStartStop(bool started);
    void updateStatusBar();
    void onSocketDataReceived(const QJsonObject &data);

public slots:
    void onMessageReceived(const QString &user, const QString &message, const QDateTime &timestamp);
    void onTicketUpdated(const QJsonObject &ticketInfo);

private:
    void setupUI();
    void applyStyle();
    void setupConnections();
    void initSocketConnection(); // 需要其他成员实现Socket连接

private:
    Ui::MainWindow *ui;
    VideoWidget *videoWidget;
    DeviceDataWidget *deviceDataWidget;
    ChatWidget *chatWidget;
    TicketManager *ticketManager;
    SocketClient *socketClient; // 由其他成员实现的Socket客户端

    QString currentUsername;
    QString currentTicketId;
    bool isVideoActive;
    bool isAudioActive;
    bool isScreenSharing;
    bool isRecording;
    QTimer *statusTimer;
};
#endif // MAINWINDOW_H
