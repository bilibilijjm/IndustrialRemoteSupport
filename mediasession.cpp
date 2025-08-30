#include "mediasession.h"
#include <QJsonDocument>
#include <QJsonObject>

MediaSession::MediaSession(const QString &sessionId, QObject *parent)
    : QObject(parent), m_sessionId(sessionId)
{
}

bool MediaSession::addClient(const QString &clientId)
{
    if (m_clients.contains(clientId)) {
        return false;
    }

    m_clients.insert(clientId);
    emit clientAdded(clientId);
    return true;
}

bool MediaSession::removeClient(const QString &clientId)
{
    if (!m_clients.contains(clientId)) {
        return false;
    }

    m_clients.remove(clientId);
    emit clientRemoved(clientId);
    return true;
}

void MediaSession::broadcastToClients(const QString &messageType, const QJsonObject &data)
{
    QJsonObject message;
    message["type"] = messageType;
    message["data"] = data;

    QByteArray jsonData = QJsonDocument(message).toJson();

    for (QTcpSocket *socket : m_clientSockets) {
        if (socket && socket->state() == QTcpSocket::ConnectedState) {
            socket->write(jsonData);
        }
    }
}

void MediaSession::broadcastMediaData(const QString &fromClient, const QString &mediaType, const QByteArray &data)
{
    QJsonObject message;
    message["type"] = "media_data";
    message["media_type"] = mediaType;
    message["from_client"] = fromClient;
    message["data"] = QString(data.toBase64());

    QByteArray jsonData = QJsonDocument(message).toJson();

    // 向会话中所有其他客户端转发媒体数据
    for (const auto &clientId : m_clients) {
        if (clientId != fromClient && m_clientSockets.contains(clientId)) {
            QTcpSocket *socket = m_clientSockets.value(clientId);
            if (socket && socket->state() == QTcpSocket::ConnectedState) {
                socket->write(jsonData);
            }
        }
    }
}
