#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "LoginDialog.h"
#include <QStatusBar>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QSplitter>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , videoWidget(new VideoWidget(this))
    , deviceDataWidget(new DeviceDataWidget(this))
    , chatWidget(new ChatWidget(this))
    , ticketManager(new TicketManager(this))
    , socketClient(nullptr) // 需要其他成员初始化
    , isVideoActive(false)
    , isAudioActive(false)
    , isScreenSharing(false)
    , isRecording(false)
{
    ui->setupUi(this);

    // 应用科技风样式
    applyStyle();

    // 初始化UI布局
    setupUI();

    // 初始化Socket连接 - 需要其他成员实现
    initSocketConnection();

    // 显示登录对话框
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        currentUsername = loginDialog.getUsername();
        onLoginSuccess(currentUsername);
    } else {
        QTimer::singleShot(0, this, &QMainWindow::close);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 设置中心部件
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 左侧区域：视频和设备数据
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->addWidget(videoWidget);
    leftLayout->addWidget(deviceDataWidget);
    leftPanel->setLayout(leftLayout);

    // 右侧区域：聊天和工单管理
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(chatWidget);
    rightLayout->addWidget(ticketManager);
    rightPanel->setLayout(rightLayout);

    // 设置分割器
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 70);
    mainSplitter->setStretchFactor(1, 30);

    setCentralWidget(mainSplitter);

    // 设置状态栏
    statusBar()->showMessage(tr("就绪"));
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    statusTimer->start(1000);

    // 连接信号和槽
    setupConnections();
}

void MainWindow::applyStyle()
{
    // 加载科技风样式表 - 需要替换为实际路径
    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
    }
}

void MainWindow::setupConnections()
{
    // 连接工单管理信号
    connect(ticketManager, &TicketManager::createTicket, this, &MainWindow::onCreateTicket);
    connect(ticketManager, &TicketManager::joinTicket, this, &MainWindow::onJoinTicket);
    connect(ticketManager, &TicketManager::ticketCreated, this, &MainWindow::onTicketCreated);

    // 连接视频组件信号
    connect(videoWidget, &VideoWidget::videoStateChanged, this, &MainWindow::onVideoStartStop);
    connect(videoWidget, &VideoWidget::audioStateChanged, this, &MainWindow::onAudioStartStop);
    connect(videoWidget, &VideoWidget::screenShareStateChanged, this, &MainWindow::onScreenShareStartStop);
    connect(videoWidget, &VideoWidget::recordingStateChanged, this, &MainWindow::onRecordingStartStop);

    // 连接聊天组件信号
    connect(chatWidget, &ChatWidget::messageSent, this, &MainWindow::onSendMessage);

    // 连接设备数据信号
    connect(deviceDataWidget, &DeviceDataWidget::dataRequested, this, [this]() {
        // 请求设备数据 - 需要其他成员实现设备数据获取
        if (socketClient) {
            // 发送数据请求命令
            QJsonObject request;
            request["type"] = "data_request";
            request["ticket_id"] = currentTicketId;
            // socketClient->sendData(request); // 需要其他成员实现
        }
    });

    // Socket信号连接
    if (socketClient) {
        connect(socketClient, &SocketClient::dataReceived, this, &MainWindow::onSocketDataReceived);
    }
}

// 临时模拟代码
void MainWindow::initSocketConnection()
{
    // 暂时使用模拟数据测试UI
    QTimer::singleShot(1000, this, [this]() {
        statusBar()->showMessage(tr("模拟连接成功"));

        // 模拟接收数据
        QJsonObject simulatedData;
        simulatedData["type"] = "device_data";
        simulatedData["temperature"] = 25.5;
        simulatedData["pressure"] = 101.3;
        // onDeviceDataReceived(simulatedData);
    });
}
//void MainWindow::initSocketConnection()
//{
    // 需要其他成员实现Socket客户端初始化和连接
    // socketClient = new SocketClient(this);
    // connect(socketClient, &SocketClient::dataReceived, this, &MainWindow::onDataReceived);
    // connect(socketClient, &SocketClient::connected, this, [this]() {
    //     statusBar()->showMessage(tr("已连接到服务器"));
    // });
    // connect(socketClient, &SocketClient::disconnected, this, [this]() {
    //     statusBar()->showMessage(tr("与服务器断开连接"));
    // });

    // 模拟Socket连接成功
//    statusBar()->showMessage(tr("已连接到服务器(模拟)"));
//}

void MainWindow::onSocketDataReceived(const QJsonObject &data)
{
    QString type = data["type"].toString();

    if (type == "chat_message") {
        // 处理聊天消息
        QString user = data["user"].toString();
        QString message = data["message"].toString();
        QDateTime timestamp = QDateTime::fromString(data["timestamp"].toString(), Qt::ISODate);

        if (chatWidget) {
            bool isSelf = (user == currentUsername);
            chatWidget->addMessage(user, message, timestamp, isSelf);
        }
    }
    else if (type == "device_data") {
        // 处理设备数据
        onDeviceDataReceived(data);
    }
    else if (type == "ticket_update") {
        // 处理工单更新
        onTicketUpdated(data);
    }
    else if (type == "video_status") {
        // 处理视频状态更新
        QString user = data["user"].toString();
        bool status = data["status"].toBool();
        if (user != currentUsername) { // 只处理其他用户的状态
            // 更新UI显示其他用户的视频状态
        }
    }
    else if (type == "audio_status") {
        // 处理音频状态更新
        QString user = data["user"].toString();
        bool status = data["status"].toBool();
        if (user != currentUsername) {
            // 更新UI显示其他用户的音频状态
        }
    }
    else {
        qDebug() << "收到未知类型的数据:" << type;
    }
}

void MainWindow::onLoginSuccess(const QString &username)
{
    setWindowTitle(tr("工业现场远程专家支持系统 - 工厂客户端 - 用户: %1").arg(username));
    statusBar()->showMessage(tr("登录成功，欢迎 %1").arg(username), 3000);
}

void MainWindow::onCreateTicket()
{
    // 创建工单逻辑 - 需要其他成员实现与服务器的交互
    QString ticketId = "T" + QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    onTicketCreated(ticketId);

    // 模拟：通过Socket发送工单创建请求
    if (socketClient) {
        QJsonObject ticketRequest;
        ticketRequest["type"] = "create_ticket";
        ticketRequest["user"] = currentUsername;
        ticketRequest["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        // socketClient->sendData(ticketRequest); // 需要其他成员实现
    }
}

void MainWindow::onJoinTicket()
{
    QString ticketId = ticketManager->getSelectedTicketId();
    if (ticketId.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请选择要加入的工单"));
        return;
    }

    currentTicketId = ticketId;
    statusBar()->showMessage(tr("已加入工单: %1").arg(ticketId));

    // 通知服务器加入工单 - 需要其他成员实现
    if (socketClient) {
        QJsonObject joinRequest;
        joinRequest["type"] = "join_ticket";
        joinRequest["ticket_id"] = ticketId;
        joinRequest["user"] = currentUsername;
        // socketClient->sendData(joinRequest); // 需要其他成员实现
    }
}

void MainWindow::onTicketCreated(const QString &ticketId)
{
    currentTicketId = ticketId;
    ticketManager->addTicket(ticketId);
    statusBar()->showMessage(tr("工单创建成功: %1").arg(ticketId));
}

void MainWindow::onVideoStartStop(bool started)
{
    isVideoActive = started;
    statusBar()->showMessage(started ? tr("视频已开启") : tr("视频已关闭"), 2000);

    // 通知服务器视频状态变化 - 需要其他成员实现
    if (socketClient && !currentTicketId.isEmpty()) {
        QJsonObject videoStatus;
        videoStatus["type"] = "video_status";
        videoStatus["ticket_id"] = currentTicketId;
        videoStatus["user"] = currentUsername;
        videoStatus["status"] = started;
        videoStatus["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        // socketClient->sendData(videoStatus); // 需要其他成员实现
    }
}

void MainWindow::onAudioStartStop(bool started)
{
    isAudioActive = started;
    statusBar()->showMessage(started ? tr("音频已开启") : tr("音频已关闭"), 2000);

    // 通知服务器音频状态变化 - 需要其他成员实现
    if (socketClient && !currentTicketId.isEmpty()) {
        QJsonObject audioStatus;
        audioStatus["type"] = "audio_status";
        audioStatus["ticket_id"] = currentTicketId;
        audioStatus["user"] = currentUsername;
        audioStatus["status"] = started;
        audioStatus["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        // socketClient->sendData(audioStatus); // 需要其他成员实现
    }
}

void MainWindow::onScreenShareStartStop(bool started)
{
    isScreenSharing = started;
    statusBar()->showMessage(started ? tr("屏幕共享已开启") : tr("屏幕共享已关闭"), 2000);

    // 通知服务器屏幕共享状态变化 - 需要其他成员实现
    if (socketClient && !currentTicketId.isEmpty()) {
        QJsonObject screenShareStatus;
        screenShareStatus["type"] = "screenshare_status";
        screenShareStatus["ticket_id"] = currentTicketId;
        screenShareStatus["user"] = currentUsername;
        screenShareStatus["status"] = started;
        screenShareStatus["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        // socketClient->sendData(screenShareStatus); // 需要其他成员实现
    }

    // 注意：屏幕共享功能需要额外的权限和实现
    // VideoWidget中暂时没有实现屏幕共享，这里需要处理
    if (started) {
        QMessageBox::information(this, tr("提示"),
                               tr("屏幕共享功能需要额外实现，当前为模拟状态"));
    }
}

void MainWindow::onRecordingStartStop(bool started)
{
    isRecording = started;
    QString message = started ? tr("录制已开始") : tr("录制已停止");
    statusBar()->showMessage(message, 2000);

    // 通知服务器录制状态变化 - 需要其他成员实现
    if (socketClient && !currentTicketId.isEmpty()) {
        QJsonObject recordingStatus;
        recordingStatus["type"] = "recording_status";
        recordingStatus["ticket_id"] = currentTicketId;
        recordingStatus["user"] = currentUsername;
        recordingStatus["status"] = started;
        recordingStatus["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        // socketClient->sendData(recordingStatus); // 需要其他成员实现
    }

    // 如果是停止录制，可以在这里处理录制文件
    if (!started && videoWidget) {
        // 获取录制文件路径并保存到知识库 - 需要其他成员实现数据库操作
        QString recordingPath = videoWidget->getCurrentRecordingFile();
        if (!recordingPath.isEmpty()) {
            qDebug() << "录制文件保存至:" << recordingPath;
            // databaseManager->saveRecording(currentTicketId, recordingPath, ...); // 需要其他成员实现
        }
    }
}

void MainWindow::onSendMessage(const QString &message)
{
    if (message.trimmed().isEmpty()) {
        return;
    }

    // 本地显示消息
    if (chatWidget) {
        chatWidget->addMessage(currentUsername, message, QDateTime::currentDateTime(), true);
    }

    // 发送消息到服务器 - 需要其他成员实现
    if (socketClient && !currentTicketId.isEmpty()) {
        QJsonObject chatMessage;
        chatMessage["type"] = "chat_message";
        chatMessage["ticket_id"] = currentTicketId;
        chatMessage["user"] = currentUsername;
        chatMessage["message"] = message;
        chatMessage["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        // socketClient->sendData(chatMessage); // 需要其他成员实现
    }

    // 保存消息到本地数据库 - 需要其他成员实现
    // if (databaseManager) {
    //     databaseManager->saveMessage(currentTicketId, currentUsername, message, QDateTime::currentDateTime());
    // }
}

// 接收消息的槽函数（由Socket客户端触发）
void MainWindow::onMessageReceived(const QString &user, const QString &message, const QDateTime &timestamp)
{
    // 注意：这个函数需要与Socket客户端的dataReceived信号连接
    if (chatWidget) {
        bool isSelf = (user == currentUsername);
        chatWidget->addMessage(user, message, timestamp, isSelf);
    }
}

// 设备数据更新槽函数
void MainWindow::onDeviceDataReceived(const QJsonObject &data)
{
    if (deviceDataWidget) {
        deviceDataWidget->updateDeviceData(data);
    }
}

// 工单更新槽函数
void MainWindow::onTicketUpdated(const QJsonObject &ticketInfo)
{
    if (ticketManager) {
        QString ticketId = ticketInfo["ticket_id"].toString();
        QString status = ticketInfo["status"].toString();

        // 更新工单状态 - 需要根据实际工单管理器的接口调整
        ticketManager->updateTicketStatus(ticketId, status);
    }
}

void MainWindow::updateStatusBar()
{
    QString status = tr("用户: %1 | ").arg(currentUsername);

    if (!currentTicketId.isEmpty()) {
        status += tr("工单: %1 | ").arg(currentTicketId);
    }

    status += tr("视频: %1 | 音频: %2 | 录屏: %3 | 录制: %4")
        .arg(isVideoActive ? tr("开") : tr("关"))
        .arg(isAudioActive ? tr("开") : tr("关"))
        .arg(isScreenSharing ? tr("开") : tr("关"))
        .arg(isRecording ? tr("开") : tr("关"));

    statusBar()->showMessage(status);
}
