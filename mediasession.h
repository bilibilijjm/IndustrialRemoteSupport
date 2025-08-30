//媒体会话管理//
#ifndef MEDIASESSION_H
#define MEDIASESSION_H

#include <QObject>
#include <QSet>
#include <QTcpSocket>
#include <QMap>

class MediaSession : public QObject
{
    Q_OBJECT
public:
    explicit MediaSession(const QString &sessionId, QObject *parent = nullptr);

    QString sessionId() const { return m_sessionId; }
    int clientCount() const { return m_clients.size(); }

    bool addClient(const QString &clientId);
    bool removeClient(const QString &clientId);

    void broadcastToClients(const QString &messageType, const QJsonObject &data);
    void broadcastMediaData(const QString &fromClient, const QString &mediaType, const QByteArray &data);

signals:
    void clientAdded(const QString &clientId);
    void clientRemoved(const QString &clientId);

private:
    QString m_sessionId;
    QSet<QString> m_clients;
    QMap<QString, QTcpSocket*> m_clientSockets; // 客户端ID -> socket
};

#endif // MEDIASESSION_H
