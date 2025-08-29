#ifndef UDPBUS_H
#define UDPBUS_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QJsonObject>

class UdpBus : public QObject {
    Q_OBJECT
public:
    explicit UdpBus(QObject* parent=nullptr);
    ~UdpBus();

    // 单例（可选）：方便每个 Widget 直接拿来用
    static UdpBus* instance(QObject* parent = nullptr);

    // 绑定本地端口（默认任意网卡 IPv4）
    // share=true 时使用 ShareAddress|ReuseAddressHint，方便多进程/多实例监听同一端口
    bool bind(quint16 port,
              const QHostAddress& addr = QHostAddress::AnyIPv4,
              bool share = true);

    // 组播（多播）
    bool joinMulticast(const QHostAddress& group,
                       const QNetworkInterface& iface = QNetworkInterface());
    bool leaveMulticast(const QHostAddress& group,
                        const QNetworkInterface& iface = QNetworkInterface());

    // 发送原始数据/JSON（点对点）
    qint64 sendRaw(const QByteArray& data, const QHostAddress& host, quint16 port);
    qint64 sendJson(const QJsonObject& obj, const QHostAddress& host, quint16 port);

    // 广播（仅 IPv4）
    qint64 broadcastRaw(const QByteArray& data, quint16 port);
    qint64 broadcastJson(const QJsonObject& obj, quint16 port);

    // 小工具
    quint16 localPort() const;

signals:
    // 收到任意 UDP 数据报
    void datagramReceived(const QHostAddress& from, quint16 port, const QByteArray& data);
    // 如果内容是 JSON，则额外发一个
    void jsonReceived(const QHostAddress& from, quint16 port, const QJsonObject& obj);
    // 简易“频道”机制：若 JSON 里含 "ch": "<channel>" 则发这个信号
    void channelReceived(const QString& channel,
                         const QHostAddress& from, quint16 port,
                         const QJsonObject& obj);

    void errorOccurred(const QString& message);

private slots:
    void onReadyRead();
    void onErrorOld(QAbstractSocket::SocketError err); // Qt 5.12

private:
    QUdpSocket* sock_ = nullptr;
    static UdpBus* g_instance;              // 单例指针
    static constexpr quint32 kMaxDatagram = 64 * 1024; // 保护上限（注意 MTU）
};

#endif // UDPBUS_H
