#include "socketclient.h"
#include "protocal.h"

#include <QJsonDocument>
#include <QDataStream>
#include <QAbstractSocket>
#include <QDebug>

SocketClient::SocketClient(QObject* parent)
    : QObject(parent),
      socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::readyRead,    this, &SocketClient::onReadyRead);
    connect(socket, &QTcpSocket::connected,    this, &SocketClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &SocketClient::onDisconnected);

    // Qt 5.12 用旧信号 error(...)
    connect(socket,
            static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this,
            &SocketClient::onError);
}

void SocketClient::connectTo(const QString& host, quint16 port)
{
    socket->connectToHost(host, port);
}

// 统一的可靠发送 JSON（4字节大端长度 + JSON 载荷）
bool SocketClient::sendJson(const QJsonObject& obj)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        emit errorOccurred(tr("未连接到服务器"));
        return false;
    }

    const QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    if (static_cast<quint32>(json.size()) > kMaxFrame) {
        emit errorOccurred(tr("JSON 消息过大（%1 字节）").arg(json.size()));
        return false;
    }

    const QByteArray frame = Proto::pack(obj);        // [len-be32] + json
    const qint64 n = socket->write(frame);
    if (n != frame.size()) {
        emit errorOccurred(tr("发送失败：%1").arg(socket->errorString()));
        return false;
    }
    return true;
}

void SocketClient::join(const QString& user, const QString& room)
{
    user_ = user;
    room_ = room;

    QJsonObject obj{
        {"type", "join"},
        {"user", user_},
        {"room", room_}
    };
    (void)sendJson(obj);
}

void SocketClient::sendChat(const QString& text, const QString& toUser)
{
    QJsonObject obj{
        {"type", "chat"},
        {"text", text}
    };
    if (!toUser.isEmpty()) obj.insert("to", toUser);

    (void)sendJson(obj);
}

// 读取并拆包（支持一批多帧）
void SocketClient::onReadyRead()
{
    recvBuf.append(socket->readAll());

    QJsonObject one;
    while (Proto::tryUnpack(recvBuf, &one)) {
        emit messageArrived(one);
    }
}

void SocketClient::onConnected()
{
    emit connected();
    qDebug() << "Socket connected";
}

void SocketClient::onDisconnected()
{
    emit disconnected();
    qDebug() << "Socket disconnected";
}

void SocketClient::onError(QAbstractSocket::SocketError)
{
    emit errorOccurred(socket->errorString());
    qWarning() << "Socket error:" << socket->errorString();
}
