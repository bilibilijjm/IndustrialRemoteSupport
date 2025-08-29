#ifndef COMMHUB_H
#define COMMHUB_H

#include <QObject>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>

class SocketClient;
class UdpBus;

class CommHub : public QObject {
    Q_OBJECT
public:
    explicit CommHub(QObject* parent = nullptr);

    // TCP 控制面:工单、登录、设备控制和数据落库。
    void startTcp(const QString& host, quint16 port); // 连接
    void tcpJoin(const QString& user, const QString& room = "lobby");
    void sendControl(const QJsonObject& obj);         // 发可靠控制消息

    // UDP 音视频、设备状态广播。
    bool bindUdp(quint16 port, const QHostAddress& addr = QHostAddress::AnyIPv4, bool share=true);
    bool joinMulticast(const QHostAddress& group);
    qint64 sendUdpJson(const QJsonObject& obj, const QHostAddress& host, quint16 port);
    qint64 sendUdpBroadcast(const QJsonObject& obj, quint16 port);

signals:
    // TCP 事件
    void tcpConnected();
    void tcpDisconnected();
    void tcpError(const QString& msg);
    void controlReceived(const QJsonObject& obj);

    // UDP 事件
    void udpJsonReceived(const QHostAddress& from, quint16 port, const QJsonObject& obj);

private:
    SocketClient* tcp_ = nullptr;
    UdpBus*       udp_ = nullptr;
};

#endif // COMMHUB_H
