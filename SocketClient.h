#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>

class SocketClient : public QObject {
    Q_OBJECT
public:
    explicit SocketClient(QObject* parent=nullptr);

    void connectTo(const QString& host, quint16 port);
    void join(const QString& user, const QString& room = "lobby");
    void sendChat(const QString& text, const QString& toUser = QString());
    bool sendJson(const QJsonObject& obj);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& msg);
    void messageArrived(const QJsonObject& obj); // 收到任何 JSON

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError);

private:
    QTcpSocket sock_;
    QByteArray buffer_;
    QString user_;
    QString room_;

private:
    QTcpSocket *socket = nullptr;
    QByteArray  recvBuf;
    quint32     pendingSize = 0;
    static constexpr quint32 kMaxFrame = 64 * 1024; // 64KiB

};

#endif // SOCKETCLIENT_H
