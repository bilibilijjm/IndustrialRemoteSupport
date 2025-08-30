#ifndef PROTOCAL_H
#define PROTOCAL_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>

namespace Proto {
static const quint32 kMaxFrame = 64 * 1024; // 64KiB

// 打包： [4字节长度(大端)] + JSON
inline QByteArray pack(const QJsonObject& obj) {
    const QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QByteArray frame;
    QDataStream ds(&frame, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << static_cast<quint32>(payload.size());  // 写4字节长度
    frame.append(payload);                        // 直接追加载荷
    return frame;
}


// 拆包：从 buffer 中尽量取出一帧 JSON（成功返回 true，并把 out 赋值）
inline bool tryUnpack(QByteArray& buffer, QJsonObject* out) {
    if (buffer.size() < 4) return false;
    QDataStream ds(buffer.left(4));
    ds.setByteOrder(QDataStream::BigEndian);
    quint32 len = 0; ds >> len;
    if (len == 0 || len > kMaxFrame) return false;
    if (buffer.size() < 4 + int(len)) return false;

    const QByteArray payload = buffer.mid(4, len);
    buffer.remove(0, 4 + len);

    QJsonParseError e{};
    const auto doc = QJsonDocument::fromJson(payload, &e);
    if (e.error != QJsonParseError::NoError || !doc.isObject()) return false;
    *out = doc.object();
    return true;
}
} // namespace Proto
#endif // PROTOCAL_H

