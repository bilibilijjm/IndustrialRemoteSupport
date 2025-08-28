//Socket客户端封装
//SocketClient.h - 需要其他成员实现
//临时实现
#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>

class SocketClient : public QObject
{
    Q_OBJECT

public:
    explicit SocketClient(QObject *parent = nullptr);
    ~SocketClient();  // 声明析构函数

    bool connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;
    bool sendData(const QJsonObject &data);

signals:
    void connected();
    void disconnected();
    void dataReceived(const QJsonObject &data);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *socket;
};

#endif // SOCKETCLIENT_H
