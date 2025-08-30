//媒体服务器实现//
#ifndef MEDIASERVER_H
#define MEDIASERVER_H

#include <QTcpServer>
#include <QMap>
#include <QSet>
#include "mediasession.h"

class MediaServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MediaServer(QObject *parent = nullptr);
    ~MediaServer();

    bool startServer(quint16 port);
    void stopServer();

    // 管理会话
    QString createSession(const QString &clientId);
    bool joinSession(const QString &sessionId, const QString &clientId);
    void leaveSession(const QString &sessionId, const QString &clientId);

signals:
    void sessionCreated(const QString &sessionId);
    void clientJoined(const QString &sessionId, const QString &clientId);
    void clientLeft(const QString &sessionId, const QString &clientId);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected();

private:
    QMap<QString, MediaSession*> m_sessions; // 会话ID -> 会话对象
    QMap<QTcpSocket*, QString> m_clientSessions; // 客户端socket -> 会话ID
    QMap<QTcpSocket*, QString> m_clientIds; // 客户端socket -> 客户端ID
    QSet<QTcpSocket*> m_clients; // 所有连接的客户端
};

#endif // MEDIASERVER_H
