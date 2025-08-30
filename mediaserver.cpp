// MediaServer.cpp
#include "media.h"
#include "mediasession.h"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>

MediaServer::MediaServer(QObject *parent) : QTcpServer(parent)
{
}

MediaServer::~MediaServer()
{
    stopServer();
}

bool MediaServer::startServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        qWarning() << "无法启动媒体服务器:" << errorString();
        return false;
    }
    qInfo() << "媒体服务器已启动，端口:" << port;
    return true;
}

void MediaServer::stopServer()
{
    for (auto socket : m_clients) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(1000);
        }
        socket->deleteLater();
    }
    m_clients.clear();

    for (auto session : m_sessions) {
        delete session;
    }
    m_sessions.clear();

    close();
}

void MediaServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *clientSocket = new QTcpSocket(this);
    if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "设置socket描述符失败:" << clientSocket->errorString();
        delete clientSocket;
        return;
    }

    m_clients.insert(clientSocket);
    connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
        // 处理客户端消息
        QByteArray data = clientSocket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qWarning() << "无效的JSON数据";
            return;
        }

        QJsonObject json = doc.object();
        QString type = json.value("type").toString();

        if (type == "create_session") {
            QString clientId = json.value("client_id").toString();
            QString sessionId = createSession(clientId);

            QJsonObject response;
            response["type"] = "session_created";
            response["session_id"] = sessionId;

            clientSocket->write(QJsonDocument(response).toJson());
            m_clientSessions[clientSocket] = sessionId;
            m_clientIds[clientSocket] = clientId;
        }
        else if (type == "join_session") {
            QString sessionId = json.value("session_id").toString();
            QString clientId = json.value("client_id").toString();

            if (joinSession(sessionId, clientId)) {
                m_clientSessions[clientSocket] = sessionId;
                m_clientIds[clientSocket] = clientId;

                QJsonObject response;
                response["type"] = "session_joined";
                response["session_id"] = sessionId;
                clientSocket->write(QJsonDocument(response).toJson());

                // 通知会话中的所有客户端有新成员加入
                if (m_sessions.contains(sessionId)) {
                    m_sessions[sessionId]->broadcastToClients(
                        QString("client_joined"),
                        QJsonObject{{"client_id", clientId}}
                    );
                }
            } else {
                QJsonObject response;
                response["type"] = "error";
                response["message"] = "加入会话失败";
                clientSocket->write(QJsonDocument(response).toJson());
            }
        }
        else if (type == "media_data") {
            QString sessionId = m_clientSessions.value(clientSocket);
            if (!sessionId.isEmpty() && m_sessions.contains(sessionId)) {
                // 转发媒体数据到会话中的其他客户端
                QByteArray mediaData = QByteArray::fromBase64(json.value("data").toString().toLatin1());
                QString mediaType = json.value("media_type").toString();

                m_sessions[sessionId]->broadcastMediaData(
                    m_clientIds[clientSocket], mediaType, mediaData
                );
            }
        }
    });

    connect(clientSocket, &QTcpSocket::disconnected, this, &MediaServer::onClientDisconnected);
}

void MediaServer::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QString sessionId = m_clientSessions.value(clientSocket);
    QString clientId = m_clientIds.value(clientSocket);

    if (!sessionId.isEmpty() && m_sessions.contains(sessionId)) {
        leaveSession(sessionId, clientId);

        // 通知会话中的其他客户端有成员离开
        m_sessions[sessionId]->broadcastToClients(
            "client_left",
            QJsonObject{{"client_id", clientId}}
        );
    }

    m_clientSessions.remove(clientSocket);
    m_clientIds.remove(clientSocket);
    m_clients.remove(clientSocket);
    clientSocket->deleteLater();
}

QString MediaServer::createSession(const QString &clientId)
{
    QString sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    MediaSession *session = new MediaSession(sessionId, this);
    m_sessions.insert(sessionId, session);

    emit sessionCreated(sessionId);
    return sessionId;
}

bool MediaServer::joinSession(const QString &sessionId, const QString &clientId)
{
    if (!m_sessions.contains(sessionId)) {
        return false;
    }

    return m_sessions[sessionId]->addClient(clientId);
}

void MediaServer::leaveSession(const QString &sessionId, const QString &clientId)
{
    if (m_sessions.contains(sessionId)) {
        m_sessions[sessionId]->removeClient(clientId);

        // 如果会话中没有客户端了，删除会话
        if (m_sessions[sessionId]->clientCount() == 0) {
            delete m_sessions[sessionId];
            m_sessions.remove(sessionId);
        }
    }
}
