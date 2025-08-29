#include "CommHub.h"
#include "SocketClient.h"
#include "UdpBus.h"

CommHub::CommHub(QObject* parent) : QObject(parent) {
    // 复用全局 UdpBus
    udp_ = UdpBus::instance(this);

    // 建 TCP 客户端
    tcp_ = new SocketClient(this);

    // 转发 TCP 信号
    connect(tcp_, &SocketClient::connected,    this, &CommHub::tcpConnected);
    connect(tcp_, &SocketClient::disconnected, this, &CommHub::tcpDisconnected);
    connect(tcp_, &SocketClient::errorOccurred,this, &CommHub::tcpError);
    connect(tcp_, &SocketClient::messageArrived, this, &CommHub::controlReceived);

    // 转发 UDP 信号
    connect(udp_, &UdpBus::jsonReceived, this, [this](const QHostAddress& from, quint16 port, const QJsonObject& o){
        emit udpJsonReceived(from, port, o);
    });
}

void CommHub::startTcp(const QString& host, quint16 port) {
    tcp_->connectTo(host, port);
}
void CommHub::tcpJoin(const QString& user, const QString& room) {
    tcp_->join(user, room);
}
void CommHub::sendControl(const QJsonObject& obj) {
    // 统一从这里走 TCP（内部会做打包）
    // 你已有 sendChat(...)，也可以直接复用；这里发任意 JSON 更灵活
    // 简单做法：用 SocketClient 的 sendChat 包个壳，或给 SocketClient 增加 sendJson()
    QJsonObject wrapped = obj;
    if (!wrapped.contains("type")) wrapped["type"] = "ctrl";
    // 这里复用 sendChat 作为通道（也可在 SocketClient 里加 sendJson）
    // 避免歧义：我们发一个有 "type":"ctrl" 的消息
    tcp_->sendChat(QString::fromUtf8(QJsonDocument(wrapped).toJson(QJsonDocument::Compact)));
}

bool CommHub::bindUdp(quint16 port, const QHostAddress& addr, bool share) {
    return udp_->bind(port, addr, share);
}
bool CommHub::joinMulticast(const QHostAddress& group) {
    return udp_->joinMulticast(group);
}
qint64 CommHub::sendUdpJson(const QJsonObject& obj, const QHostAddress& host, quint16 port) {
    return udp_->sendJson(obj, host, port);
}
qint64 CommHub::sendUdpBroadcast(const QJsonObject& obj, quint16 port) {
    return udp_->broadcastJson(obj, port);
}
