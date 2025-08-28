//Socket客户端实现
//SocketClient.cpp - 需要其他成员实现
//临时实现
#include "SocketClient.h"
#include <QJsonDocument>
#include <QHostAddress>
#include <QDebug>
#include <QDataStream>

SocketClient::SocketClient(QObject *parent)
    : QObject(parent), socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::readyRead, this, &SocketClient::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &SocketClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &SocketClient::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &SocketClient::onError);
}

SocketClient::~SocketClient()
{
    disconnectFromServer();
    socket->deleteLater();  // 确保socket被正确清理
    qDebug() << "SocketClient destructor called";
}

bool SocketClient::connectToServer(const QString &host, quint16 port)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    socket->connectToHost(host, port);
    return socket->waitForConnected(3000);
}

void SocketClient::disconnectFromServer()
{
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(1000);
        }
    }
}

bool SocketClient::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

bool SocketClient::sendData(const QJsonObject &data)
{
    if (!isConnected()) {
        emit errorOccurred(tr("未连接到服务器"));
        return false;
    }

    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    // 添加数据长度前缀
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << quint32(jsonData.size());
    packet.append(jsonData);

    qint64 bytesWritten = socket->write(packet);
    return bytesWritten == packet.size();
}

void SocketClient::onReadyRead()
{
    while (socket->bytesAvailable() > 0) {
        static quint32 packetSize = 0;

        if (packetSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint32)) {
                return;
            }
            QDataStream stream(socket);
            stream >> packetSize;
        }

        if (socket->bytesAvailable() < packetSize) {
            return;
        }

        QByteArray jsonData = socket->read(packetSize);
        packetSize = 0;

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
            emit dataReceived(doc.object());
        }
    }
}

void SocketClient::onConnected()
{
    emit connected();
    qDebug() << "Connected to server";
}

void SocketClient::onDisconnected()
{
    emit disconnected();
    qDebug() << "Disconnected from server";
}

void SocketClient::onError(QAbstractSocket::SocketError error)
{
    QString errorMsg = socket->errorString();
    emit errorOccurred(errorMsg);
    qWarning() << "Socket error:" << errorMsg;
}
