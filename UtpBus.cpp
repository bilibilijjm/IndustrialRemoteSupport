#include "UdpBus.h"
#include <QJsonDocument>
#include <QDataStream>
#include <QCoreApplication>


UdpBus* UdpBus::g_instance = nullptr;

UdpBus::UdpBus(QObject* parent) : QObject(parent), sock_(new QUdpSocket(this))
{
    connect(sock_, &QUdpSocket::readyRead, this, &UdpBus::onReadyRead);

    connect(sock_,
            static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this,
            &UdpBus::onErrorOld);
}

UdpBus::~UdpBus() {}

UdpBus* UdpBus::instance(QObject* parent)
{
    if (!g_instance) g_instance = new UdpBus(parent ? parent : qApp);
    return g_instance;
}

bool UdpBus::bind(quint16 port, const QHostAddress& addr, bool share)
{
    QUdpSocket::BindMode mode = QUdpSocket::DefaultForPlatform;
    if (share) mode |= QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint;
    const bool ok = sock_->bind(addr, port, mode);
    if (!ok) emit errorOccurred(sock_->errorString());
    return ok;
}

bool UdpBus::joinMulticast(const QHostAddress& group, const QNetworkInterface& iface)
{
    const bool ok = sock_->joinMulticastGroup(group, iface);
    if (!ok) emit errorOccurred(sock_->errorString());
    return ok;
}

bool UdpBus::leaveMulticast(const QHostAddress& group, const QNetworkInterface& iface)
{
    const bool ok = sock_->leaveMulticastGroup(group, iface);
    if (!ok) emit errorOccurred(sock_->errorString());
    return ok;
}

qint64 UdpBus::sendRaw(const QByteArray& data, const QHostAddress& host, quint16 port)
{
    if (data.size() > (int)kMaxDatagram) {
        emit errorOccurred(tr("数据报过大：%1 字节").arg(data.size()));
        return -1;
    }
    return sock_->writeDatagram(data, host, port);
}

qint64 UdpBus::sendJson(const QJsonObject& obj, const QHostAddress& host, quint16 port)
{
    const QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    return sendRaw(payload, host, port);
}

qint64 UdpBus::broadcastRaw(const QByteArray& data, quint16 port)
{
    sock_->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    return sendRaw(data, QHostAddress::Broadcast, port);
}

qint64 UdpBus::broadcastJson(const QJsonObject& obj, quint16 port)
{
    const QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    return broadcastRaw(payload, port);
}

quint16 UdpBus::localPort() const
{
    return sock_->localPort();
}

void UdpBus::onReadyRead()
{
    while (sock_->hasPendingDatagrams()) {
        QHostAddress from;
        quint16 fromPort = 0;
        QByteArray buf;
        buf.resize(int(sock_->pendingDatagramSize()));
        const qint64 n = sock_->readDatagram(buf.data(), buf.size(), &from, &fromPort);
        if (n < 0) {
            emit errorOccurred(sock_->errorString());
            return;
        }
        buf.truncate(int(n));
        emit datagramReceived(from, fromPort, buf);

        // 尝试解析 JSON
        QJsonParseError e{};
        const auto doc = QJsonDocument::fromJson(buf, &e);
        if (e.error == QJsonParseError::NoError && doc.isObject()) {
            const auto obj = doc.object();
            emit jsonReceived(from, fromPort, obj);
            if (obj.contains(QStringLiteral("ch"))) {
                emit channelReceived(obj.value(QStringLiteral("ch")).toString(),
                                     from, fromPort, obj);
            }
        }
    }
}

void UdpBus::onErrorOld(QAbstractSocket::SocketError)
{
    emit errorOccurred(sock_->errorString());
}
