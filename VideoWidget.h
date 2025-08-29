#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
class QWebEngineView;

class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent=nullptr);
    // roomId: 房间号（整数的字符串，如 "1234"）
    // display: 昵称
    // janusUrl: 例如 "ws://127.0.0.1:8188" 或 "https://janus.conf.meetecho.com/janus"
    void startCall(const QString& roomId, const QString& display, const QString& janusUrl);

private:
    QWebEngineView *view_ = nullptr;
    void grantMediaPermissions();
};

#endif // VIDEOWIDGET_H
